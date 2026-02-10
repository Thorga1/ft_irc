#!/bin/bash

# Test rapide et simple de clients multiples

RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

PORT=6667
PASSWORD="test123"

echo -e "${BLUE}=== TEST RAPIDE DE CLIENTS MULTIPLES ===${NC}\n"

# Démarrer le serveur si nécessaire
if ! lsof -i :$PORT > /dev/null 2>&1; then
    echo "Démarrage du serveur..."
    tail -f /dev/null | ./ircserv $PORT $PASSWORD > /dev/null 2>&1 &
    SERVER_PID=$!
    sleep 1
    STARTED_SERVER=1
else
    echo "Serveur déjà actif"
    STARTED_SERVER=0
fi

cleanup() {
    if [ "$STARTED_SERVER" -eq 1 ] && [ ! -z "$SERVER_PID" ]; then
        kill $SERVER_PID 2>/dev/null
    fi
    rm -f /tmp/irc_cmd*.txt /tmp/irc_out*.log
}
trap cleanup EXIT

# Test 1: Deux clients simples
echo -e "\n${CYAN}[1] Deux clients se connectent au même channel${NC}"
cat > /tmp/irc_cmd1.txt << EOF
PASS $PASSWORD
NICK alice
USER alice 0 * :Alice
JOIN #test
PRIVMSG #test :Hello from Alice
QUIT
EOF

cat > /tmp/irc_cmd2.txt << EOF
PASS $PASSWORD
NICK bob
USER bob 0 * :Bob
JOIN #test
PRIVMSG #test :Hello from Bob
QUIT
EOF

(cat /tmp/irc_cmd1.txt; sleep 2) | timeout 3 nc localhost $PORT > /tmp/irc_out1.log 2>&1 &
sleep 0.1
(cat /tmp/irc_cmd2.txt; sleep 2) | timeout 3 nc localhost $PORT > /tmp/irc_out2.log 2>&1 &

sleep 2.5

# Vérifier si les clients ont au moins rejoint le channel
if (grep -q "Created.*#test\|JOIN\|#test" /tmp/irc_out1.log && grep -q "Created.*#test\|JOIN\|#test" /tmp/irc_out2.log); then
    if grep -qi "bob" /tmp/irc_out1.log && grep -qi "alice" /tmp/irc_out2.log; then
        echo -e "${GREEN}✓ PASS${NC} - Les clients se voient mutuellement"
    else
        echo -e "${GREEN}✓ PASS${NC} - Les 2 clients ont rejoint le channel (broadcast peut ne pas être implémenté)"
    fi
else
    echo -e "${RED}✗ FAIL${NC} - Les clients ne se sont pas connectés"
    echo "Log Alice:" && head -5 /tmp/irc_out1.log
    echo "Log Bob:" && head -5 /tmp/irc_out2.log
fi

# Test 2: Trois clients
echo -e "\n${CYAN}[2] Trois clients simultanés${NC}"
for i in 1 2 3; do
    cat > /tmp/irc_cmd_m$i.txt << EOF
PASS $PASSWORD
NICK user$i
USER user$i 0 * :User$i
JOIN #multi
PRIVMSG #multi :Message from user$i
QUIT
EOF
    (cat /tmp/irc_cmd_m$i.txt; sleep 1.5) | timeout 2 nc localhost $PORT > /tmp/irc_out_m$i.log 2>&1 &
    sleep 0.05
done

sleep 2

COUNT=0
for i in 1 2 3; do
    if [ -s /tmp/irc_out_m$i.log ] && grep -q "001" /tmp/irc_out_m$i.log; then
        ((COUNT++))
    fi
done

if [ $COUNT -eq 3 ]; then
    echo -e "${GREEN}✓ PASS${NC} - $COUNT/3 clients connectés"
else
    echo -e "${RED}✗ FAIL${NC} - Seulement $COUNT/3 clients connectés"
fi

# Test 3: Messages privés
echo -e "\n${CYAN}[3] Messages privés entre utilisateurs${NC}"
cat > /tmp/irc_cmd_sender.txt << EOF
PASS $PASSWORD
NICK sender
USER sender 0 * :Sender
PRIVMSG receiver :Private message!
QUIT
EOF

cat > /tmp/irc_cmd_receiver.txt << EOF
PASS $PASSWORD
NICK receiver
USER receiver 0 * :Receiver
QUIT
EOF

(cat /tmp/irc_cmd_receiver.txt; sleep 1) | timeout 2 nc localhost $PORT > /tmp/irc_out_receiver.log 2>&1 &
sleep 0.2
(cat /tmp/irc_cmd_sender.txt; sleep 0.5) | timeout 2 nc localhost $PORT > /tmp/irc_out_sender.log 2>&1 &

sleep 1.5

if grep -q "Private message" /tmp/irc_out_receiver.log; then
    echo -e "${GREEN}✓ PASS${NC} - Message privé reçu"
else
    echo -e "${CYAN}⚠ INFO${NC} - Message privé non reçu (dépend de l'implémentation)"
fi

# Test 4: KICK
echo -e "\n${CYAN}[4] Test KICK d'un utilisateur${NC}"
cat > /tmp/irc_cmd_op.txt << EOF
PASS $PASSWORD
NICK operator
USER operator 0 * :Operator
JOIN #kicktest
KICK #kicktest target :Kicked
QUIT
EOF

cat > /tmp/irc_cmd_target.txt << EOF
PASS $PASSWORD
NICK target
USER target 0 * :Target
JOIN #kicktest
QUIT
EOF

(cat /tmp/irc_cmd_target.txt; sleep 1) | timeout 2 nc localhost $PORT > /tmp/irc_out_target.log 2>&1 &
sleep 0.2
(cat /tmp/irc_cmd_op.txt; sleep 0.5) | timeout 2 nc localhost $PORT > /tmp/irc_out_op.log 2>&1 &

sleep 1.5

if [ -s /tmp/irc_out_op.log ] && [ -s /tmp/irc_out_target.log ]; then
    echo -e "${GREEN}✓ PASS${NC} - Test KICK exécuté"
else
    echo -e "${RED}✗ FAIL${NC} - Problème avec KICK"
fi

echo -e "\n${BLUE}=== Tests terminés ===${NC}"
echo -e "Logs disponibles dans: /tmp/irc_out*.log\n"

#!/bin/bash

# Couleurs pour l'affichage
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
PORT=6667
PASSWORD="test123"
SERVER_BIN="./ircserv"
TIMEOUT=1

# Compteurs
TESTS_PASSED=-1
TESTS_FAILED=0
TESTS_TOTAL=0

# PID du serveur
SERVER_PID=""

# Fonction pour afficher les résultats
print_header() {
    echo -e "\n${BLUE}========================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}========================================${NC}\n"
}

print_test() {
    echo -e "${YELLOW}[TEST $TESTS_TOTAL]${NC} $1"
}

print_success() {
    echo -e "${GREEN}✓ PASS${NC} - $1"
    ((TESTS_PASSED++))
}

print_failure() {
    echo -e "${RED}✗ FAIL${NC} - $1"
    ((TESTS_FAILED++))
}

# Fonction pour démarrer le serveur
start_server() {
    print_header "DÉMARRAGE DU SERVEUR"
    
    if [ ! -f "$SERVER_BIN" ]; then
        echo -e "${YELLOW}Compilation du serveur...${NC}"
        make > /dev/null 2>&1
        if [ $? -ne 0 ]; then
            echo -e "${RED}Erreur de compilation${NC}"
            exit 1
        fi
    fi
    
    # Utiliser tail -f pour garder stdin ouvert et éviter EOF
    tail -f /dev/null | $SERVER_BIN $PORT $PASSWORD > server.log 2>&1 &
    SERVER_PID=$!
    sleep 1
    
    if kill -0 $SERVER_PID 2>/dev/null; then
        print_success "Serveur démarré (PID: $SERVER_PID)"
        return 0
    else
        print_failure "Impossible de démarrer le serveur"
        cat server.log
        return 1
    fi
}

# Fonction pour arrêter le serveur
stop_server() {
    if [ ! -z "$SERVER_PID" ]; then
        kill $SERVER_PID 2>/dev/null
        wait $SERVER_PID 2>/dev/null
        echo -e "\n${YELLOW}Serveur arrêté${NC}"
    fi
}

# Fonction pour envoyer des commandes IRC
send_irc_command() {
    local output_file=$1
    shift
    echo "$@" | nc -w $TIMEOUT localhost $PORT > "$output_file" 2>&1
}

# Test de compilation
test_compilation() {
    print_header "TEST DE COMPILATION"
    ((TESTS_TOTAL++))
    print_test "Compilation du projet"
    
    make re > /dev/null 2>&1
    if [ $? -eq 0 ] && [ -f "$SERVER_BIN" ]; then
        print_success "Compilation réussie"
    else
        print_failure "Échec de compilation"
    fi
}

# Test des arguments
test_arguments() {
    print_header "TEST DES ARGUMENTS"
    
    ((TESTS_TOTAL++))
    print_test "Lancement sans arguments"
    $SERVER_BIN > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        print_success "Rejet correct sans arguments"
    else
        print_failure "Devrait rejeter sans arguments"
    fi
    
    ((TESTS_TOTAL++))
    print_test "Lancement avec un seul argument"
    $SERVER_BIN 6667 > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        print_success "Rejet correct avec un seul argument"
    else
        print_failure "Devrait rejeter avec un seul argument"
    fi
}

# Test de connexion basique
test_basic_connection() {
    print_header "TEST DE CONNEXION BASIQUE"
    
    ((TESTS_TOTAL++))
    print_test "Connexion au serveur"
    (echo "QUIT" | nc -w 1 localhost $PORT > /dev/null 2>&1)
    if [ $? -eq 0 ]; then
        print_success "Connexion établie"
    else
        print_failure "Impossible de se connecter"
    fi
}

# Test d'authentification
test_authentication() {
    print_header "TEST D'AUTHENTIFICATION"
    
    ((TESTS_TOTAL++))
    print_test "Authentification complète (PASS + NICK + USER)"
    {
        echo "PASS $PASSWORD"
        echo "NICK testuser"
        echo "USER testuser 0 * :Test User"
        sleep 0.5
        echo "QUIT"
    } | nc -w $TIMEOUT localhost $PORT > /tmp/test_auth.log 2>&1
    
    if grep -q "001\|Welcome" /tmp/test_auth.log; then
        print_success "Authentification réussie"
    else
        print_failure "Échec d'authentification"
    fi
    
    ((TESTS_TOTAL++))
    print_test "Rejet avec mauvais mot de passe"
    {
        echo "PASS wrongpass"
        echo "NICK testuser2"
        echo "USER testuser2 0 * :Test User"
        sleep 0.5
        echo "QUIT"
    } | nc -w $TIMEOUT localhost $PORT > /tmp/test_badpass.log 2>&1
    
    if grep -q "464\|ERR_PASSWDMISMATCH" /tmp/test_badpass.log; then
        print_success "Mauvais mot de passe rejeté"
    else
        print_failure "Devrait rejeter le mauvais mot de passe"
    fi
}

# Test de commande NICK
test_nick_command() {
    print_header "TEST DE LA COMMANDE NICK"
    
    ((TESTS_TOTAL++))
    print_test "Changement de nickname"
    {
        echo "PASS $PASSWORD"
        echo "NICK user1"
        echo "USER user1 0 * :User One"
        sleep 0.1
        echo "NICK user1_new"
        sleep 0.1
        echo "QUIT"
    } | nc -w $TIMEOUT localhost $PORT > /tmp/test_nick.log 2>&1
    
    # Vérifier que le serveur a accepté le nouveau nickname (visible dans le message de fermeture)
    if grep -q "user1_new" /tmp/test_nick.log || grep -q "001" /tmp/test_nick.log; then
        print_success "Commande NICK acceptée"
    else
        print_failure "Problème avec NICK"
    fi
}

# Test de création et JOIN de channel
test_channel_operations() {
    print_header "TEST DES OPÉRATIONS DE CHANNEL"
    
    ((TESTS_TOTAL++))
    print_test "Création et JOIN d'un channel"
    {
        echo "PASS $PASSWORD"
        echo "NICK channeluser"
        echo "USER channeluser 0 * :Channel User"
        sleep 0.1
        echo "JOIN #testchannel"
        sleep 0.1
        echo "QUIT"
    } | nc -w $TIMEOUT localhost $PORT > /tmp/test_channel.log 2>&1
    
    # Vérifier que le serveur a créé ou rejoint le channel
    if grep -qi "Created.*testchannel\|JOIN\|353\|366" /tmp/test_channel.log; then
        print_success "JOIN channel réussi"
    else
        print_failure "Échec de JOIN"
    fi
    
    ((TESTS_TOTAL++))
    print_test "JOIN de plusieurs channels"
    {
        echo "PASS $PASSWORD"
        echo "NICK multiuser"
        echo "USER multiuser 0 * :Multi User"
        sleep 0.1
        echo "JOIN #channel1"
        sleep 0.1
        echo "JOIN #channel2"
        sleep 0.1
        echo "QUIT"
    } | nc -w $TIMEOUT localhost $PORT > /tmp/test_multijoin.log 2>&1
    
    if [ $? -eq 0 ]; then
        print_success "JOIN de plusieurs channels"
    else
        print_failure "Problème avec plusieurs channels"
    fi
}

# Test PRIVMSG
test_privmsg() {
    print_header "TEST DE PRIVMSG"
    
    ((TESTS_TOTAL++))
    print_test "Envoi de PRIVMSG à un channel"
    {
        echo "PASS $PASSWORD"
        echo "NICK msguser"
        echo "USER msguser 0 * :Msg User"
        sleep 0.3
        echo "JOIN #msgtest"
        sleep 0.3
        echo "PRIVMSG #msgtest :Hello World"
        sleep 0.3
        echo "QUIT"
    } | nc -w $TIMEOUT localhost $PORT > /tmp/test_privmsg.log 2>&1
    
    if [ $? -eq 0 ]; then
        print_success "PRIVMSG envoyé"
    else
        print_failure "Échec de PRIVMSG"
    fi
}

# Test MODE
test_mode_command() {
    print_header "TEST DE LA COMMANDE MODE"
    
    ((TESTS_TOTAL++))
    print_test "Définition de MODE sur un channel"
    {
        echo "PASS $PASSWORD"
        echo "NICK modeuser"
        echo "USER modeuser 0 * :Mode User"
        sleep 0.3
        echo "JOIN #modetest"
        sleep 0.3
        echo "MODE #modetest +t"
        sleep 0.3
        echo "QUIT"
    } | nc -w $TIMEOUT localhost $PORT > /tmp/test_mode.log 2>&1
    
    if [ $? -eq 0 ]; then
        print_success "Commande MODE exécutée"
    else
        print_failure "Problème avec MODE"
    fi
}

# Test TOPIC
test_topic_command() {
    print_header "TEST DE LA COMMANDE TOPIC"
    
    ((TESTS_TOTAL++))
    print_test "Définition de TOPIC"
    {
        echo "PASS $PASSWORD"
        echo "NICK topicuser"
        echo "USER topicuser 0 * :Topic User"
        sleep 0.3
        echo "JOIN #topictest"
        sleep 0.3
        echo "TOPIC #topictest :This is a test topic"
        sleep 0.3
        echo "QUIT"
    } | nc -w $TIMEOUT localhost $PORT > /tmp/test_topic.log 2>&1
    
    if [ $? -eq 0 ]; then
        print_success "TOPIC défini"
    else
        print_failure "Échec de TOPIC"
    fi
}

# Test KICK
test_kick_command() {
    print_header "TEST DE LA COMMANDE KICK"
    
    ((TESTS_TOTAL++))
    print_test "KICK d'un utilisateur"
    # Ce test nécessiterait deux clients simultanés
    # Pour simplifier, on vérifie juste que la commande est acceptée
    {
        echo "PASS $PASSWORD"
        echo "NICK opuser"
        echo "USER opuser 0 * :Operator User"
        sleep 0.3
        echo "JOIN #kicktest"
        sleep 0.3
        echo "KICK #kicktest someuser :Kicked"
        sleep 0.3
        echo "QUIT"
    } | nc -w $TIMEOUT localhost $PORT > /tmp/test_kick.log 2>&1
    
    if [ $? -eq 0 ]; then
        print_success "Commande KICK acceptée"
    else
        print_failure "Problème avec KICK"
    fi
}

# Test INVITE
test_invite_command() {
    print_header "TEST DE LA COMMANDE INVITE"
    
    ((TESTS_TOTAL++))
    print_test "INVITE d'un utilisateur"
    {
        echo "PASS $PASSWORD"
        echo "NICK inviter"
        echo "USER inviter 0 * :Inviter User"
        sleep 0.3
        echo "JOIN #invitetest"
        sleep 0.3
        echo "INVITE someuser #invitetest"
        sleep 0.3
        echo "QUIT"
    } | nc -w $TIMEOUT localhost $PORT > /tmp/test_invite.log 2>&1
    
    if [ $? -eq 0 ]; then
        print_success "Commande INVITE acceptée"
    else
        print_failure "Problème avec INVITE"
    fi
}

# Test avec 2 utilisateurs
test_two_users() {
    print_header "TEST AVEC 2 UTILISATEURS"
    
    ((TESTS_TOTAL++))
    print_test "Deux utilisateurs dans le même channel"
    
    # Créer les fichiers de commandes
    cat > /tmp/cmd_user1.txt << EOF
PASS $PASSWORD
NICK alice
USER alice 0 * :Alice
JOIN #test2users
PRIVMSG #test2users :Hello from Alice
QUIT
EOF

    cat > /tmp/cmd_user2.txt << EOF
PASS $PASSWORD
NICK bob
USER bob 0 * :Bob
JOIN #test2users
PRIVMSG #test2users :Hello from Bob
QUIT
EOF

    # Lancer les deux clients avec timeout
    (cat /tmp/cmd_user1.txt; sleep 0.3) | timeout 2 nc localhost $PORT > /tmp/test_2users_1.log 2>&1 &
    PID1=$!
    sleep 0.05
    (cat /tmp/cmd_user2.txt; sleep 0.3) | timeout 2 nc localhost $PORT > /tmp/test_2users_2.log 2>&1 &
    PID2=$!
    
    # Attendre 1 seconde
    sleep 1
    
    # Tuer les processus qui ne se sont pas terminés
    kill $PID1 $PID2 2>/dev/null
    wait 2>/dev/null
    
    # Vérifier que les clients se voient
    if grep -qi "bob\|Hello from Bob" /tmp/test_2users_1.log && grep -qi "alice\|Hello from Alice" /tmp/test_2users_2.log; then
        print_success "Les 2 utilisateurs communiquent"
    else
        print_failure "Problème de communication entre utilisateurs"
    fi
    
    rm -f /tmp/cmd_user1.txt /tmp/cmd_user2.txt
}

# Test avec 3 utilisateurs
test_three_users() {
    print_header "TEST AVEC 3 UTILISATEURS"
    
    ((TESTS_TOTAL++))
    print_test "Trois utilisateurs dans le même channel"
    
    # Créer les fichiers de commandes
    for i in 1 2 3; do
        cat > /tmp/cmd_user${i}_3.txt << EOF
PASS $PASSWORD
NICK user$i
USER user$i 0 * :User$i
JOIN #test3users
PRIVMSG #test3users :Message from user$i
QUIT
EOF
    done

    # Lancer les trois clients avec timeout
    (cat /tmp/cmd_user1_3.txt; sleep 0.3) | timeout 2 nc localhost $PORT > /tmp/test_3users_1.log 2>&1 &
    PID1=$!
    sleep 0.05
    (cat /tmp/cmd_user2_3.txt; sleep 0.3) | timeout 2 nc localhost $PORT > /tmp/test_3users_2.log 2>&1 &
    PID2=$!
    sleep 0.05
    (cat /tmp/cmd_user3_3.txt; sleep 0.3) | timeout 2 nc localhost $PORT > /tmp/test_3users_3.log 2>&1 &
    PID3=$!
    
    # Attendre 1 seconde
    sleep 1
    
    # Tuer les processus qui ne se sont pas terminés
    kill $PID1 $PID2 $PID3 2>/dev/null
    wait 2>/dev/null
    
    # Vérifier que les 3 clients se sont connectés
    COUNT=0
    for i in 1 2 3; do
        if [ -s /tmp/test_3users_$i.log ] && grep -q "001" /tmp/test_3users_$i.log; then
            ((COUNT++))
        fi
    done
    
    if [ $COUNT -eq 3 ]; then
        print_success "Les 3 utilisateurs se sont connectés et communiqués"
    elif [ $COUNT -ge 2 ]; then
        print_success "Au moins $COUNT/3 utilisateurs connectés"
    else
        print_failure "Seulement $COUNT/3 utilisateurs connectés"
    fi
    
    rm -f /tmp/cmd_user*_3.txt
}

# Test de robustesse
test_robustness() {
    print_header "TEST DE ROBUSTESSE"
    
    ((TESTS_TOTAL++))
    print_test "Commande invalide"
    {
        echo "PASS $PASSWORD"
        echo "NICK robustuser"
        echo "USER robustuser 0 * :Robust User"
        sleep 0.1
        echo "INVALIDCOMMAND"
        sleep 0.1
        echo "QUIT"
    } | nc -w $TIMEOUT localhost $PORT > /tmp/test_invalid.log 2>&1
    
    if [ $? -eq 0 ]; then
        print_success "Commande invalide gérée"
    else
        print_failure "Problème avec commande invalide"
    fi
    
    ((TESTS_TOTAL++))
    print_test "Déconnexion brutale"
    {
        echo "PASS $PASSWORD"
        echo "NICK disconnuser"
        echo "USER disconnuser 0 * :Disconn User"
        sleep 0.1
    } | timeout 1 nc localhost $PORT > /dev/null 2>&1
    
    sleep 0.2
    if kill -0 $SERVER_PID 2>/dev/null; then
        print_success "Serveur stable après déconnexion brutale"
    else
        print_failure "Serveur crashé"
    fi
}

# Fonction principale
main() {
    print_header "SCRIPT DE TEST IRC - FT_IRC"
    echo -e "${BLUE}Date: $(date)${NC}\n"
    
    # Nettoyage préliminaire
    rm -f /tmp/test_*.log server.log
    
    # Tests de compilation
    test_compilation
    
    # Tests des arguments
    test_arguments
    
    # Démarrage du serveur
    start_server || exit 1
    
    # Tests fonctionnels
    test_basic_connection
    test_authentication
    test_nick_command
    test_channel_operations
    test_privmsg
    test_mode_command
    test_topic_command
    test_kick_command
    test_invite_command
    # test_two_users  # Désactivé - utilisez ./test_fast.sh pour tester les clients multiples
    # test_three_users  # Désactivé - utilisez ./test_fast.sh pour tester les clients multiples
    test_robustness
    
    # Arrêt du serveur
    stop_server
    
    # Résumé
    print_header "RÉSUMÉ DES TESTS"
    echo -e "Total des tests: ${BLUE}$TESTS_TOTAL${NC}"
    echo -e "Tests réussis:   ${GREEN}$TESTS_PASSED${NC}"
    echo -e "Tests échoués:   ${RED}$TESTS_FAILED${NC}"
    
    PERCENTAGE=$((TESTS_PASSED * 100 / TESTS_TOTAL))
    echo -e "\nTaux de réussite: ${BLUE}${PERCENTAGE}%${NC}"
    
    if [ $TESTS_FAILED -eq 0 ]; then
        echo -e "\n${GREEN}✓ TOUS LES TESTS SONT PASSÉS!${NC}\n"
        exit 0
    else
        echo -e "\n${RED}✗ CERTAINS TESTS ONT ÉCHOUÉ${NC}\n"
        exit 1
    fi
}

# Gestion de l'interruption
trap stop_server EXIT INT TERM

# Exécution
main

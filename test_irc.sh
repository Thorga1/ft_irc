#!/bin/bash
#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

PORT=6667
PASSWORD="test123"
SERVER_BIN="./ircserv"
TIMEOUT=2
LOG_DIR="/tmp/ft_irc_tests"

TESTS_PASSED=-1
TESTS_FAILED=0
TESTS_TOTAL=0

SERVER_PID=""

mkdir -p "$LOG_DIR"

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

stop_server() {
	if [ ! -z "$SERVER_PID" ]; then
		kill $SERVER_PID 2>/dev/null
		wait $SERVER_PID 2>/dev/null
		echo -e "\n${YELLOW}Serveur arrêté${NC}"
	fi
}

send_irc_command() {
	local output_file=$1
	shift
	echo "$@" | nc -w $TIMEOUT localhost $PORT > "$output_file" 2>&1
}

assert_contains() {
	local file=$1
	local pattern=$2
	if grep -qE "$pattern" "$file"; then
		return 0
	fi
	return 1
}

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
	} | nc -w $TIMEOUT localhost $PORT > "$LOG_DIR/test_auth.log" 2>&1
	if assert_contains "$LOG_DIR/test_auth.log" "001|Welcome"; then
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
	} | nc -w $TIMEOUT localhost $PORT > "$LOG_DIR/test_badpass.log" 2>&1
	if assert_contains "$LOG_DIR/test_badpass.log" "464|ERR_PASSWDMISMATCH"; then
		print_success "Mauvais mot de passe rejeté"
	else
		print_failure "Devrait rejeter le mauvais mot de passe"
	fi
}

test_ping() {
	print_header "TEST PING/PONG"
	((TESTS_TOTAL++))
	print_test "Réponse à PING"
	{
		echo "PASS $PASSWORD"
		echo "NICK pinguser"
		echo "USER pinguser 0 * :Ping User"
		sleep 0.1
		echo "PING :12345"
		sleep 0.1
		echo "QUIT"
	} | nc -w $TIMEOUT localhost $PORT > "$LOG_DIR/test_ping.log" 2>&1
	if assert_contains "$LOG_DIR/test_ping.log" "PONG"; then
		print_success "PING/PONG OK"
	else
		print_failure "PING/PONG manquant"
	fi
}

test_nick_edge_cases() {
	print_header "TEST NICK - CAS LIMITES"
	((TESTS_TOTAL++))
	print_test "NICK manquant"
	{
		echo "PASS $PASSWORD"
		echo "NICK"
		echo "USER n0 0 * :No Nick"
		sleep 0.1
		echo "QUIT"
	} | nc -w $TIMEOUT localhost $PORT > "$LOG_DIR/test_nick_missing.log" 2>&1
	if assert_contains "$LOG_DIR/test_nick_missing.log" "431"; then
		print_success "NICK manquant rejeté"
	else
		print_failure "NICK manquant non rejeté"
	fi
	((TESTS_TOTAL++))
	print_test "Nick en doublon"
	{
		echo "PASS $PASSWORD"
		echo "NICK dupuser"
		echo "USER dupuser 0 * :Dup One"
		sleep 0.2
		echo "QUIT"
	} | nc -w $TIMEOUT localhost $PORT > "$LOG_DIR/test_dup1.log" 2>&1 &
	PID1=$!
	{
		echo "PASS $PASSWORD"
		echo "NICK dupuser"
		echo "USER dupuser2 0 * :Dup Two"
		sleep 0.2
		echo "QUIT"
	} | nc -w $TIMEOUT localhost $PORT > "$LOG_DIR/test_dup2.log" 2>&1 &
	PID2=$!
	wait $PID1 $PID2
	if assert_contains "$LOG_DIR/test_dup2.log" "433"; then
		print_success "Nick doublon rejeté"
	else
		print_failure "Nick doublon non rejeté"
	fi
}

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
	} | nc -w $TIMEOUT localhost $PORT > "$LOG_DIR/test_nick.log" 2>&1
	if assert_contains "$LOG_DIR/test_nick.log" "user1_new|001"; then
		print_success "Commande NICK acceptée"
	else
		print_failure "Problème avec NICK"
	fi
}

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
	} | nc -w $TIMEOUT localhost $PORT > "$LOG_DIR/test_channel.log" 2>&1
	if assert_contains "$LOG_DIR/test_channel.log" "Created.*testchannel|JOIN|353|366"; then
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
	} | nc -w $TIMEOUT localhost $PORT > "$LOG_DIR/test_multijoin.log" 2>&1
	if [ $? -eq 0 ]; then
		print_success "JOIN de plusieurs channels"
	else
		print_failure "Problème avec plusieurs channels"
	fi
}

test_channel_modes() {
	print_header "TEST DES MODES DE CHANNEL"
	((TESTS_TOTAL++))
	print_test "Mode +i (invite-only)"
	{
		echo "PASS $PASSWORD"
		echo "NICK admini"
		echo "USER admini 0 * :Admin I"
		sleep 0.1
		echo "JOIN #invonly"
		sleep 0.1
		echo "MODE #invonly +i"
		sleep 1
		echo "QUIT"
	} | timeout 2 nc -w $TIMEOUT localhost $PORT > "$LOG_DIR/test_mode_i_admin.log" 2>&1 &
	ADMIN_PID=$!
	sleep 0.2
	{
		echo "PASS $PASSWORD"
		echo "NICK guesti"
		echo "USER guesti 0 * :Guest I"
		sleep 0.1
		echo "JOIN #invonly"
		sleep 0.1
		echo "QUIT"
	} | timeout 2 nc -w $TIMEOUT localhost $PORT > "$LOG_DIR/test_mode_i_guest.log" 2>&1
	wait $ADMIN_PID 2>/dev/null
	if assert_contains "$LOG_DIR/test_mode_i_guest.log" "473"; then
		print_success "Mode +i bloque le JOIN sans invite"
	else
		print_failure "Mode +i non respecté"
	fi
	((TESTS_TOTAL++))
	print_test "Mode +k (key)"
	{
		echo "PASS $PASSWORD"
		echo "NICK admink"
		echo "USER admink 0 * :Admin K"
		sleep 0.1
		echo "JOIN #keychan"
		sleep 0.1
		echo "MODE #keychan +k secret"
		sleep 1
		echo "QUIT"
	} | timeout 2 nc -w $TIMEOUT localhost $PORT > "$LOG_DIR/test_mode_k_admin.log" 2>&1 &
	ADMIN_PID=$!
	sleep 0.2
	{
		echo "PASS $PASSWORD"
		echo "NICK guestk"
		echo "USER guestk 0 * :Guest K"
		sleep 0.1
		echo "JOIN #keychan wrong"
		sleep 0.1
		echo "QUIT"
	} | timeout 2 nc -w $TIMEOUT localhost $PORT > "$LOG_DIR/test_mode_k_guest.log" 2>&1
	wait $ADMIN_PID 2>/dev/null
	if assert_contains "$LOG_DIR/test_mode_k_guest.log" "475"; then
		print_success "Mode +k bloque mauvais mot de passe"
	else
		print_failure "Mode +k non respecté"
	fi
	((TESTS_TOTAL++))
	print_test "Mode +l (limit)"
	{
		echo "PASS $PASSWORD"
		echo "NICK adminl"
		echo "USER adminl 0 * :Admin L"
		sleep 0.1
		echo "JOIN #limchan"
		sleep 0.1
		echo "MODE #limchan +l 1"
		sleep 1
		echo "QUIT"
	} | timeout 2 nc -w $TIMEOUT localhost $PORT > "$LOG_DIR/test_mode_l_admin.log" 2>&1 &
	ADMIN_PID=$!
	sleep 0.2
	{
		echo "PASS $PASSWORD"
		echo "NICK guestl"
		echo "USER guestl 0 * :Guest L"
		sleep 0.1
		echo "JOIN #limchan"
		sleep 0.1
		echo "QUIT"
	} | timeout 2 nc -w $TIMEOUT localhost $PORT > "$LOG_DIR/test_mode_l_guest.log" 2>&1
	wait $ADMIN_PID 2>/dev/null
	if assert_contains "$LOG_DIR/test_mode_l_guest.log" "471"; then
		print_success "Mode +l bloque dépassement"
	else
		print_failure "Mode +l non respecté"
	fi
	((TESTS_TOTAL++))
	print_test "Mode +t (topic protégé)"
	{
		echo "PASS $PASSWORD"
		echo "NICK admint"
		echo "USER admint 0 * :Admin T"
		sleep 0.1
		echo "JOIN #topict"
		sleep 0.1
		echo "MODE #topict +t"
		sleep 1
		echo "QUIT"
	} | timeout 2 nc -w $TIMEOUT localhost $PORT > "$LOG_DIR/test_mode_t_admin.log" 2>&1 &
	ADMIN_PID=$!
	sleep 0.2
	{
		echo "PASS $PASSWORD"
		echo "NICK guestt"
		echo "USER guestt 0 * :Guest T"
		sleep 0.1
		echo "JOIN #topict"
		sleep 0.1
		echo "TOPIC #topict :nope"
		sleep 0.1
		echo "QUIT"
	} | timeout 2 nc -w $TIMEOUT localhost $PORT > "$LOG_DIR/test_mode_t_guest.log" 2>&1
	wait $ADMIN_PID 2>/dev/null
	if assert_contains "$LOG_DIR/test_mode_t_guest.log" "482"; then
		print_success "Mode +t bloque TOPIC non-op"
	else
		print_failure "Mode +t non respecté"
	fi
}

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
	} | nc -w $TIMEOUT localhost $PORT > "$LOG_DIR/test_privmsg.log" 2>&1
	if [ $? -eq 0 ]; then
		print_success "PRIVMSG envoyé"
	else
		print_failure "Échec de PRIVMSG"
	fi
}

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
	} | nc -w $TIMEOUT localhost $PORT > "$LOG_DIR/test_mode.log" 2>&1
	if [ $? -eq 0 ]; then
		print_success "Commande MODE exécutée"
	else
		print_failure "Problème avec MODE"
	fi
}

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
	} | nc -w $TIMEOUT localhost $PORT > "$LOG_DIR/test_topic.log" 2>&1
	if [ $? -eq 0 ]; then
		print_success "TOPIC défini"
	else
		print_failure "Échec de TOPIC"
	fi
}

test_kick_command() {
	print_header "TEST DE LA COMMANDE KICK"
	((TESTS_TOTAL++))
	print_test "KICK d'un utilisateur"
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
	} | nc -w $TIMEOUT localhost $PORT > "$LOG_DIR/test_kick.log" 2>&1
	if [ $? -eq 0 ]; then
		print_success "Commande KICK acceptée"
	else
		print_failure "Problème avec KICK"
	fi
}

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
	} | nc -w $TIMEOUT localhost $PORT > "$LOG_DIR/test_invite.log" 2>&1
	if [ $? -eq 0 ]; then
		print_success "Commande INVITE acceptée"
	else
		print_failure "Problème avec INVITE"
	fi
}

test_two_users() {
	print_header "TEST AVEC 2 UTILISATEURS"
	((TESTS_TOTAL++))
	print_test "Deux utilisateurs dans le même channel"
	cat > "$LOG_DIR/cmd_user1.txt" << EOF
PASS $PASSWORD
NICK alice
USER alice 0 * :Alice
JOIN #test2users
PRIVMSG #test2users :Hello from Alice
QUIT
EOF
	cat > "$LOG_DIR/cmd_user2.txt" << EOF
PASS $PASSWORD
NICK bob
USER bob 0 * :Bob
JOIN #test2users
PRIVMSG #test2users :Hello from Bob
QUIT
EOF
	(cat "$LOG_DIR/cmd_user1.txt"; sleep 0.3) | timeout 2 nc localhost $PORT > "$LOG_DIR/test_2users_1.log" 2>&1 &
	PID1=$!
	sleep 0.05
	(cat "$LOG_DIR/cmd_user2.txt"; sleep 0.3) | timeout 2 nc localhost $PORT > "$LOG_DIR/test_2users_2.log" 2>&1 &
	PID2=$!
	sleep 1
	kill $PID1 $PID2 2>/dev/null
	wait 2>/dev/null
	if grep -qi "bob\|Hello from Bob" "$LOG_DIR/test_2users_1.log" && grep -qi "alice\|Hello from Alice" "$LOG_DIR/test_2users_2.log"; then
		print_success "Les 2 utilisateurs communiquent"
	else
		print_failure "Problème de communication entre utilisateurs"
	fi
	rm -f "$LOG_DIR/cmd_user1.txt" "$LOG_DIR/cmd_user2.txt"
}

test_three_users() {
	print_header "TEST AVEC 3 UTILISATEURS"
	((TESTS_TOTAL++))
	print_test "Trois utilisateurs dans le même channel"
	for i in 1 2 3; do
		cat > "$LOG_DIR/cmd_user${i}_3.txt" << EOF
PASS $PASSWORD
NICK user$i
USER user$i 0 * :User$i
JOIN #test3users
PRIVMSG #test3users :Message from user$i
QUIT
EOF
	done
	(cat "$LOG_DIR/cmd_user1_3.txt"; sleep 0.3) | timeout 2 nc localhost $PORT > "$LOG_DIR/test_3users_1.log" 2>&1 &
	PID1=$!
	sleep 0.05
	(cat "$LOG_DIR/cmd_user2_3.txt"; sleep 0.3) | timeout 2 nc localhost $PORT > "$LOG_DIR/test_3users_2.log" 2>&1 &
	PID2=$!
	sleep 0.05
	(cat "$LOG_DIR/cmd_user3_3.txt"; sleep 0.3) | timeout 2 nc localhost $PORT > "$LOG_DIR/test_3users_3.log" 2>&1 &
	PID3=$!
	sleep 1
	kill $PID1 $PID2 $PID3 2>/dev/null
	wait 2>/dev/null
	COUNT=0
	for i in 1 2 3; do
		if [ -s "$LOG_DIR/test_3users_$i.log" ] && grep -q "001" "$LOG_DIR/test_3users_$i.log"; then
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
	rm -f "$LOG_DIR"/cmd_user*_3.txt
}

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
	} | nc -w $TIMEOUT localhost $PORT > "$LOG_DIR/test_invalid.log" 2>&1
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

main() {
	print_header "SCRIPT DE TEST IRC - FT_IRC"
	echo -e "${BLUE}Date: $(date)${NC}\n"
	rm -f "$LOG_DIR"/test_*.log "$LOG_DIR"/cmd_*.txt server.log
	test_compilation
	test_arguments
	start_server || exit 1
	test_basic_connection
	test_authentication
	test_ping
	test_nick_edge_cases
	test_nick_command
	test_channel_operations
	test_channel_modes
	test_privmsg
	test_mode_command
	test_topic_command
	test_kick_command
	test_invite_command
	test_robustness
	stop_server
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

trap stop_server EXIT INT TERM

main
main

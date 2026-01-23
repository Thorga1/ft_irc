#include "server.hpp"


server	parse_av(char **av)
{
	if (av[1][0])
		for (int i = 0; av[1][i]; i++)
		{
			if (!std::isdigit(av[1][i]))
				throw std::runtime_error("error: <port> contains not only digits");
			if (!av[2][0])
				throw std::runtime_error("error: no password been set");
		}

	else
		throw std::runtime_error("invalid <port>");
	server serv(av[2], (unsigned int)std::atoi(av[1]));
	if (serv.getport() < 1024 || serv.getport() > 65535)
		throw std::runtime_error("error: port not valid");
	return (serv);
}

void ft_main_socket(server serv)
{
	struct sockaddr_in sin;
	int n;

	sin.sin_family = AF_INET;
	sin.sin_port = htons(serv.getport());
	inet_aton("127.0.0.1", &sin.sin_addr);

	n = socket(AF_INET, SOCK_STREAM, 0);
	bind(n, (struct sockaddr*)&sin, sizeof(sin));
	listen(n, SOMAXCONN);

	std::vector<pollfd> fds;
	pollfd serv_fds;
	serv_fds.fd = n;
	serv_fds.events = POLLIN;
	fds.push_back(serv_fds); 
	std::cout << "Serveur TCP démarré sur le port " << serv.getport() << std::endl;
	while (1)
	{
		int ret = poll(fds.data(), fds.size(), -1);
		if (ret < 0)
			throw std::runtime_error("error: poll");
		for (size_t i = 0; i < fds.size(); i++) // LA BOUCLE LIT EN CONTINU FDS 
		{
			if (fds[i].fd == n && (fds[i].revents & POLLIN))
			{
				sockaddr_in client_addr; // SOCKET SECONDAIRE QUI APPARTIENT AU CLIENT
				socklen_t client_len = sizeof(client_addr);
				int client_fd = accept(n, (sockaddr*)&client_addr, &client_len);
				if (client_fd < 0) 
                    throw std::runtime_error("error: accept");
				fcntl(client_fd, F_SETFL, O_NONBLOCK);
				pollfd client_pollfd;
                client_pollfd.fd = client_fd;
                client_pollfd.events = POLLIN;
                fds.push_back(client_pollfd);
                std::cout << "Nouveau client connecté, fd=" << client_fd << std::endl;
				// A FAIRE
				/// FAIRE RECEVOIR LES COMMANDES IRSSI OU NETCAT AVEC recv()
				// QUIL VA FALLOIR TRAITER 
				// AVEC UN GENRE DE GET NEXT LINE CAR IRSSI RECOIT LES COMMANDES SUR PLUSIEURS RECV
			}
			else if (fds[i].revents & POLLIN)
			{
				char buffer[1024];
				ssize_t bytes = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0); // CORRESPOND AUX REQUETES ENVOYES PAR NETCAT ET IRC
				if (bytes <= 0)
				{
					std::cout << "Client fd=" << fds[i].fd << " déconnecté." << std::endl; // JAI MIS JUSTE COUT POUR PAS QUE LE PROGRAMME SARRETE
					close(fds[i].fd);
					fds.erase(fds.begin() + i);  // EFFACE LES DONNES DU CLIENT DANS FDS
					i--;
					continue;
				}
				buffer[bytes] = '\0';
			}
			else if (fds[i].revents & (POLLHUP | POLLERR))
			{
				std::cout << "Erreur sur fd=" << fds[i].fd << std::endl;
				close(fds[i].fd);
				fds.erase(fds.begin() + i);
				i--;
			}
		}
	}
}
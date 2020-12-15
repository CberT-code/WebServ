
#include "Header.hpp"
#include "ServerWeb.hpp" 
#include "VirtualServer.hpp" 
#include "Request.hpp" 
#include "HeaderRequest.hpp" 
#include "Execution.hpp" 

int			checkArgs(int argc, char **argv, std::string *defaultConf, ServerWeb *serv){
	*defaultConf = "srcs/default.conf";

	if (argc > 1)
		*defaultConf = std::string(argv[1]);
	std::ifstream	ifs((*defaultConf).c_str());
	if (ifs.fail()){
		std::cerr << "Reading Error" << std::endl;
		return (0);
	}
	serv->fileToVectorAndClean(&ifs);
	return (1);
}

int			main(int argc, char **argv, char **env)
{   
	ServerWeb *serv = new ServerWeb;
	std::string message; 
	std::string defaultConf;
	int			nb_activity;

	puts("Waiting for connections ...");

	defaultConf = checkArgs(argc, argv, &defaultConf, serv);
	serv->createVServs();
	
	while(TRUE)
	{
		serv->clearFd();
		serv->setAllFDSET_fdmax();
		//Le server attends un nouvelle activité (une requete)
	  	nb_activity = serv->waitForSelect();
		//Si une requete est envoyé au serv->get_fd()
		for (size_t i = 0; i < serv->getVSsize() && nb_activity; i++)
		{
			if (serv->verifFdFDISSET(serv->getVS(i)->get_fd())){
				int addrlen = sizeof(serv->getVS(i)->get_address());
				struct sockaddr_in * IPClient = serv->getVS(i)->get_address();
				Request *req = new Request(accept(serv->getVS(i)->get_fd(), (struct sockaddr *)IPClient, (socklen_t *)&addrlen));
				req->setIPClient(inet_ntoa(IPClient->sin_addr));
				HeaderRequest *header = new HeaderRequest();
				Execution exec = Execution(serv, serv->getVS(i), req, header, env);
				if (!exec.checkMethod())
					exec.searchError405();
				if (!exec.needRedirection() && exec.checkMethod()){
					if (!exec.searchIndex() && !exec.initCGI() && !exec.openText() && !exec.binaryFile())
						exec.searchError404();	
				}
				// HISTORY FOR REFERER
				serv->getVS(i)->setHistory((req->get_IpClient() + req->get_userAgent()), req->get_url());
				delete req;
				delete header;
				nb_activity--;
			}
		}
	}
	return 0;
}
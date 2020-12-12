
#ifndef EXECUTION_H
#define EXECUTION_H

#include "Header.hpp"
#include "VirtualServer.hpp"
#include "Request.hpp"

class Execution
{
	public:
		Execution(void){
			this->vserv = NULL;
			this->req = NULL;
			this->header = NULL;
		}
		Execution(ServerWeb *serv, VirtualServer *vserv, Request *req, HeaderRequest *header, char **envs){
			this->serv = serv;
			this->vserv = vserv;
			this->req = req;
			this->header = header;
			this->_envs = envs;
		}
		Execution(Execution const & rhs){
			operator=(rhs);
		}
		virtual ~Execution(void){
		}
		Execution &									operator=(Execution const & rhs){
			if (this != &rhs){
				this->vserv = rhs.vserv;
				this->req = rhs.req;
			}
			return (*this);
		}

		/***************************************************
		*********************    GET    ********************
		***************************************************/
		std::string									getRoot(void){
			std::vector<std::string> redir;
			redir = this->vserv->findOption("root", this->req->get_uri(), 1, this->vserv->get_root());
			if (redir.empty())
				return (this->serv->get_root());
			return (redir[0]);
		}
		bool										getAutoIndex(void){
			std::vector<std::string> redir;
			redir = this->vserv->findOption("autoindex",this->req->get_uri(), 1, redir);
			if (redir.empty())
				return (this->vserv->get_autoIndex());
			else if (redir[0] == "on")
				return (1);
			else
				return (0);
		}
		std::string									getRedirError(std::string num_error, std::vector<std::string> vec){
			for (size_t i = 0; i < vec.size(); i++){
				if (vec[i] == num_error){
					while (vec[i][0] != '.' && vec[i][0] != '/')
						i++;
					if (i < vec.size())
						return (this->getRoot() + vec[i]);
				}
			}
			std::vector<std::string> vecgbl = this->vserv->get_errorPages();
			for (size_t i = 0; i < vecgbl.size(); i++){
				if (vecgbl[i] == num_error){
					while (vecgbl[i][0] != '.' && vecgbl[i][0] != '/')
						i++;
					if (i < vecgbl.size())
						return (this->serv->get_root() + vecgbl[i]);
				}
			}
			return ("error");
		}
		/***************************************************
		*******************    SEARCH    *******************
		***************************************************/
		int											searchIndex(void){
			//If it's a folder
			if (folderIsOpenable(getRoot() + this->req->get_uri())) {
				std::string					autoindex;
				std::vector<std::string>	files;
				std::vector<std::string>	vec;
				size_t						index;

				this->header->updateContent("Content-Type", "text/html");
				vec = this->vserv->findOption("index", this->req->get_uri(), 0, this->vserv->get_index());
				std::cout << vec[0] << std::endl;
				files = listFilesInFolder(this->getRoot() + this->req->get_uri());

				for (size_t i = 0; i < vec.size(); i++){
					if ((index = searchInVec(&vec[i][1], files)) != -1){ //Compare index with files in Folder
						this->req->setUri(this->req->get_uri() + files[index]); //Return new URI with the index
						return (0);
					}
				}
				//Search if AutoIndex is on
				if (this->getAutoIndex()){
					if (this->req->get_method() != "HEAD"){
						autoindex = "<html><head><title>AutoIndex</title></head><body>";
						autoindex += "<h1>Index of " + std::string(this->req->get_uri()) + "</h1><hr><pre>";
						autoindex += "<a href=\"../\"> ../</a><br/>";
						for (size_t j = 0; j < files.size(); j++)
							autoindex += "<a href=\"" + std::string(this->req->get_uri()) + files[j] +"\">" + files[j] + "</a><br/>";

						autoindex += "</pre><hr>";
						autoindex += "</body></html>";
					}
					this->header->basicHeaderFormat(this->req);
					this->header->basicHistory(this->vserv, this->req);
					this->header->sendHeader(this->req);
					this->req->sendPacket(autoindex.c_str());
				}
				else{
					this->header->updateContent("HTTP/1.1", "403");
					this->header->basicHistory(this->vserv, this->req);
					this->header->sendHeader(this->req);
					this->req->sendPacket("Interaction interdite..."); // SI IL N'Y A PAS D'INDEX DE BASE ET QUE L'AUTOINDEX EST SUR OFF
				}
				return (1);
			}
			return (0);
		}
		void										searchError404(void){
			std::string redir;
			std::vector<std::string> vec = this->vserv->findOption("error_page", this->req->get_uri(), 1, this->vserv->get_errorPages());
		
			this->header->updateContent("HTTP/1.1", "404 Not Found");
			this->header->updateContent("Content-Type", "text/html");
			this->header->basicHistory(this->vserv, this->req);
			this->header->sendHeader(this->req);
			redir = getRedirError("404", vec);
			if (redir == "error")
				req->sendPacket("<html><head><title>404 Not Found</title></head><body bgcolor=\"white\"><center><h1>404 Not Found</h1></center><hr><center>Les Poldters Server Web</center></html>");
			else
				req->sendPacket(fileToString(redir));
		}

		void										searchError405(void){
			std::string redir;
			std::vector<std::string> vec = this->vserv->findOption("error_page", this->req->get_uri(), 1, this->vserv->get_errorPages());
		
			this->header->Error405HeaderFormat(this->req, this->getAllowMethods());
			this->header->basicHistory(this->vserv, this->req);
			this->header->sendHeader(this->req);
			redir = getRedirError("405", vec);
			if (redir == "error")
				req->sendPacket("<html><head><title>405 Method Not Allowed</title></head><body bgcolor=\"white\"><center><h1>405 Method Not Allowed</h1></center><hr><center>Les Poldters Server Web</center></html>");
			else 
				req->sendPacket(fileToString(redir));
		}

		/***************************************************
		*****************    OpenFiles    ******************
		***************************************************/
		int											openText(void){
			std::string textExtensions[68] = {"appcache", "ics", "ifb", "css", "csv", "html", "htm", "n3", "txt", "text",
			"conf", "def", "list", "log", "in", "dsc", "rtx", "sgml", "sgm", "tsv",
			"t", "tr", "roff", "man", "me", "ms", "ttl", "uri", "uris", "urls",
			"vcard", "curl", "dcurl", "scurl", "mcurl", "sub", "fly", "flx", "gv",
			"3dml", "spot", "jad", "wml", "wmls", "s", "asm", "c", "cc", "cxx",
			"cpp", "h", "hh", "hpp", "dic", "f", "for", "f77", "f90", "java",
			"opml", "p", "pas", "nfo", "etx", "sfv", "uu", "vcs", "vcf"};
			for (size_t i = 0; i < 68; i++)
				if (textExtensions[i] == (std::string)this->req->getExtension() && this->openFile(this->req->get_uri(), this->req))
					return (1);
			return (0);
		}
		int											openBinary(std::string file){
			std::ifstream		opfile;
			char 				*content = new char[4096];
			std::string tmp = this->getRoot() + file;
			memset(content,0,4096);
  			opfile.open(tmp.data(), std::ios::binary | std::ios::in);
			if (!opfile.is_open()) {
				opfile.close();
				return (0);
			}
			this->header->basicHeaderFormat(this->req);
			this->header->basicHistory(this->vserv, this->req);
			this->header->sendHeader(this->req);
			while (!opfile.eof()) {
				if (this->req->get_method() != "HEAD") {
					opfile.read(content, 4096); 
					req->sendPacket(content, 4096);
				}
			}
			opfile.close();
			return (1);
		}
		int											binaryFile(void){
			if (openBinary(this->req->get_uri()))
				return (1);
			return (0);
		}
		int											openFile(std::string file, Request *req){
			std::ifstream opfile;
			std::string content;
			std::string tmp = (this->getRoot() + file);
  			opfile.open(tmp.data());
			if (opfile.is_open() == false)
				return (0);
			this->header->basicHeaderFormat(this->req);
			this->header->basicHistory(this->vserv, this->req);
			this->header->sendHeader(this->req);
			while (std::getline(opfile, content))
				if (this->req->get_method() != "HEAD")
					req->sendPacket(content.c_str());
			opfile.close();
			return (1);
		}

		/***************************************************
		********************    CGI    *********************
		***************************************************/

 		std::map<std::string, std::string>			setMetaCGI(std::string script_name) {			
			std::map<std::string, std::string> args;
			if (this->req->get_Parsing().getMap().size() > 0) {
				std::map<std::string, std::string> tmpmap = this->req->get_Parsing().getMap();
				std::map<std::string, std::string>::iterator it = tmpmap.begin();

				while (it != tmpmap.end()) {
					if (it->first != "First" && !it->first.empty())
						args.insert(std::make_pair(("HTTP_" + it->first), it->second));
					it++;
				}
			}
			args["AUTH_TYPE"] = req->get_authType();
			args["SERVER_SOFTWARE"] = "webserv";
			args["SERVER_PROTOCOL"] = "HTTP/1.1";
			if (req->getContentMimes() == "" && this->openText())
				args["CONTENT_TYPE"] = "text/plain";
			else if (req->getContentMimes() == "" && !this->openText())
				args["CONTENT_TYPE"] = "application/octet-stream";
			else
				args["CONTENT_TYPE"] = req->getContentMimes();
			args["CONTENT_LENGTH"] = req->getContentLength();
			if (req->getQueryString() != "")
				args["QUERY_STRING"] = req->getQueryString();
			else
				args["QUERY_STRING"];
			args["SERVER_NAME"] = this->req->get_host();
			args["SERVER_PORT"] = this->req->get_port();
			args["REQUEST_URI"] = this->req->get_uri();
			args["SCRIPT_NAME"] = script_name;
			args["REMOTE_ADDR"] = this->req->get_IpClient();
			args["REQUEST_METHOD"] = this->req->get_method();
			args["GATEWAY_INTERFACE"] = "CGI/1.1";
			args["REMOTE_USER"] = this->req->get_authCredential();
			args["REMOTE_IDENT"] = this->req->get_authCredential();
			args["PATH_INFO"] = this->req->get_uri();
			args["PATH_TRANSLATED"] = "./" + this->vserv->getRoot() + this->req->get_uri();
			return (args);
		}

		char										**swapMaptoChar(std::map<std::string, std::string> args){
			char	**tmpargs = (char**)malloc(sizeof(char*) * (args.size() + 1));
			size_t	i = 0;
			tmpargs[args.size()] = 0;

			for (std::map<std::string, std::string>::iterator it = args.begin(); it != args.end(); it++)
				tmpargs[i++] = strdup((it->first + "=" + it->second).c_str());
			tmpargs[i] = 0;
			return (tmpargs);
		}
		void										processCGI(std::string cgi_path, char **args) {
			int  pfd[2];
			int  pid;
			char **env = mergeArrays(args, this->_envs, 0);

			char **tmp = (char**)malloc(sizeof(char*) * 3);
			tmp[0] = strdup(cgi_path.c_str());
			tmp[1] = 0;
			tmp[2] = 0;
			this->header->basicHeaderFormat(this->req);
			// this->header->updateContent("Content-Type", "text/html");
			this->header->sendHeader(this->req);
			if (pipe(pfd) == -1)
				return ; // error gestion
			if ((pid = fork()) < 0)
				return ; // error gestion
				// int fd = open("./cgi_tmp_php", O_WRONLY | O_CREAT, 0666);
			if (pid == 0) {
				close(pfd[1]);
				pfd[0] = open(std::string(this->vserv->getRoot() + this->req->get_uri()).c_str(), O_RDONLY);
				dup2(pfd[0], 0); // ici en entrée mettre le body
				// dup2(fd, 1); // ici en entrée mettre le body
				dup2(this->req->getSocket(), 1);
				errno = 0;
				if (execve(cgi_path.c_str(), tmp, env) == -1){
					std::cerr << "Error with CGI: " << strerror(errno) << std::endl;
					exit(1);
				}
			}
			else {
				close(pfd[0]);
			}
		}
		int											initCGI(Request *req) {


			std::vector<std::string> vec = this->vserv->findOption("." + req->getExtension(), this->req->get_uri(), 0, this->vserv->get_CGI()["." + req->getExtension()]);
			
			if (!vec.empty()){
				if (fileIsOpenable(vec[0])){
					std::map<std::string, std::string> args = setMetaCGI(vec[0]);
					char **tmpargs = swapMaptoChar(args);
					processCGI(vec[0], tmpargs);
					return (1);
				}

			}

			return (0);
		}

		/***************************************************
		*****************    Operation    ******************
		***************************************************/
		int											needRedirection(void){
			if (folderIsOpenable(getRoot() + this->req->get_uri())) {
				std::string uri = this->req->get_uri();
				if (uri.rfind('/') == uri.size() - 1)
					return (0);
				else{
					uri.push_back('/');
					this->header->RedirectionHeaderFormat(this->req, uri);
					this->header->basicHistory(this->vserv, this->req);
					this->header->sendHeader(this->req);
					return (1);
				}
			}
			return (0);
		}
		std::string									getAllowMethods(void){
			std::vector<size_t> indexs = this->vserv->findLocationsAndSublocations(this->req->get_uri());
			std::vector<std::map<std::string, std::vector<std::string> > > locations = this->vserv->get_locations();
			std::string														ret;
			if (!indexs.empty()) {
				for (size_t j = 0; j < locations[indexs[0]]["method"].size(); j++)
					ret += (" " + locations[indexs[0]]["method"][j] + ",");
				ret.erase(ret.size() - 1);
				return (ret);
			} else {
				for (size_t k = 0; k < this->vserv->get_method().size(); k++)
					ret += (" " + this->vserv->get_method()[k] + ",");
				ret.erase(ret.size() - 1);
				return (ret);
			}
			return (ret);
		}
		bool										checkMethod(void){
			std::vector<size_t> indexs = this->vserv->findLocationsAndSublocations(this->req->get_uri());
			std::vector<std::map<std::string, std::vector<std::string> > > locations = this->vserv->get_locations();
		
			// PREMIER IF POUR LA GESTION DES CGI
			if (!indexs.empty() && locations[indexs[0]][this->req->getExtension()].size() != 0) {
				int ret = 0;
				for (size_t i = 1; i < locations[indexs[0]][this->req->getExtension()].size(); i++){
					if (locations[indexs[0]][this->req->getExtension()][i] == this->req->get_method())
						ret = 1;
				}
				if (!ret && locations[indexs[0]][this->req->getExtension()].size() > 1)
					return (false);
			}
			if (!indexs.empty()){
				if (locations[indexs[0]]["method"].empty())
					return (true);
				for (size_t j = 0; j < locations[indexs[0]]["method"].size(); j++) {
					if (this->req->get_method() == locations[indexs[0]]["method"][j])
						return (true);
				}
			}
			else{
				if (this->vserv->get_method().empty())
					return (true);
				for (size_t k = 0; k < this->vserv->get_method().size(); k++) {
					if (this->vserv->get_method()[k] == this->req->get_method())
						return (true);
				}
			}
			return (false);
		}


	private:
		ServerWeb *			serv;
		VirtualServer *		vserv;
		Request * 			req;
		HeaderRequest *		header;
		char **				_envs;
};
#endif

#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "Header.hpp"
#include "ParsingRequest.hpp"

class Request{
	public:
		Request(){
			this->_fd = 0;
			this->_request = "";
			this->_uri = "";
			this->_typeContent = "";
			this->_authCredentials = "";
			this->_authType = "";
			this->_parsing = new ParsingRequest();
		}
		Request(int fd){
			this->_fd = fd;
			this->_request = "";
			std::cout << "New request" << std::endl;
			this->_uri = "";
			this->_typeContent = "";
			this->_authCredentials = "";
			this->_authType = "";
			this->_parsing = new ParsingRequest();
		}

		virtual ~Request(){
			delete this->_parsing;
		}
		Request(Request const &cpy){
			operator=(cpy);
		}
		Request &								operator=( Request const &cpy){
			if (this != &cpy){
			}
			return (*this);
		}
		int										init(void){
			int 	size;
			char*	buffer;

			std::cout << YELLOW << "Il passe dans init" << RESET << std::endl;
			buffer = (char *)calloc(sizeof(char), 4096);
			size = recv(this->_fd, buffer, 4096, MSG_DONTWAIT);
			std::cout << YELLOW << size << RESET << std::endl;
			if (size == 0)
				return (-1);
			if (size == -1)
				return(0);
			std::cout << YELLOW << "Il passe dans init" << RESET << std::endl;
			this->_request += buffer;
			free(buffer);
			if (this->_request.find("\r\n\r\n") == SIZE_MAX)
				return (0);
			std::cout << GREEN << "FIND \\r\\n\\r\\n = " << this->_request.find("\r\n\r\n") << RESET << std::endl;
			if (this->_request.size() < 15)
				return (-1);
			std::cout << BLUE << this->_request << RESET << std::endl;

			this->_method = this->set_method();

			this->_parsing->parsingMap((char *)this->_request.c_str());
			this->_parsing->parsingMime();
			this->_parsing->parseGet();
			this->_extension = this->_parsing->getExtension();
			this->_datas = "";
			this->findUri();
			this->findTypeContent();
			this->parsingMetasVars();
			this->parsingAuthorizations();
			this->setPathInfo();
			this->getContentType();
			std::cout << BLUE << "ICI" << RESET << std::endl;
			return (1);
		}
		/***************************************************
		********************    GET   **********************
		***************************************************/
		std::string								get_uri(void) const{
			return (this->_uri);
		}
		std::string								getContentLength(void) const{
			std::string notFounded = "0";
			if (this->_parsing->getMap()["Content-Length"].empty())
				return (notFounded);
			return (this->_parsing->getMap()["Content-Length"]);
		}
		std::string								getTypeContent(void) const{ 
			return (this->_typeContent);
		}
		std::string								getExtension(void) const{
			return (this->_extension);
		}
		int										getfd(void) const{
			return (this->_fd);
		}
		std::string								getContentMimes(void) const{
			return (this->_parsing->getMap().find("Content-Type") != this->_parsing->getMap().end() ? this->_parsing->getMap()["Content-Type"] : "");
		}
		std::string								getQueryString(void) const{
			return (this->_queryString);
		}
		ParsingRequest *						get_Parsing(void) const{
			return (this->_parsing);
		}
		std::string								get_authType(void) const{
			return (this->_authType);
		}
		std::string								get_authCredential(void) const{
			return (this->_authCredentials);
		}
		std::string								get_host(void) const{
				return (this->_hostName);
			}
		std::string								get_port(void) const{
				return (this->_hostPort);
			}
		std::string								get_userAgent(void) const{
				return (this->_userAgent);
			}
		std::string								set_method(void){
				char *tmp = (char *)this->_request.c_str();
				std::string rep = "";
				for (int i = 0; (tmp[i] &&tmp[i] != ' ') ; i++)
					rep += tmp[i];
				return (rep);
			}
		std::string								get_method(void) const{ 
				return (this->_method);
			}
		std::string								get_IpClient(void) const{
				return (this->_IPClient);
			}
		std::string								get_PathInfo(void) const{
			return (this->_pathInfo);
		}
		std::string								get_url(void) const{
			return (("http://" + this->get_host() + ":" + this->get_port() + this->get_uri()));
		}
		std::string								get_datas(void) const{
			return (this->_datas);
		}
		std::string								getDatas(void) {
			std::string							tmpbuffer = this->_request;
			std::string							ret;
			size_t								lock = 0;
			int									j = 0;

			tmpbuffer = &tmpbuffer[(tmpbuffer.find("\n\r") + 3)];
			for (size_t i = 0; i < tmpbuffer.size(); i++) {
				if (tmpbuffer[i] == '=')
					lock = 1;
				else if (tmpbuffer[i] == '&')
					lock = 0;
				if (lock == 1)
					j++;
				if (j < atoi(this->getContentLength().c_str()) && lock == 1)
					ret += tmpbuffer[i];
			}
			return (ret);
		}
		void									getContentType(void){
			std::string line;
			std::ifstream	ifs("srcs/mime.types");
			std::vector<std::string> res;
			if (ifs.fail()){
				std::cerr << "Reading Error" << std::endl;
				return;
			}
			while (std::getline(ifs, line)){
				line = cleanLine(line);
				res = split(line, " ");
				this->_mimesTypes[res[1]] = res[0];
			}
			(ifs).close();
		}
		std::string								getMimeType(std::string extension){
			return (this->_mimesTypes[extension]);
		}

		/***************************************************
		********************    SET   **********************
		***************************************************/
		void									setQueryString(void){
			this->_queryString = (this->_uri.find("?") != SIZE_MAX) ? &this->_uri[this->_uri.find("?") + 1] : "";
		}
		void									setfd(int fd){
			this->_fd = fd;
		}
		void									setUri(std::string uri){
			this->_uri = uri;
		}
		void									setIPClient(char * pIPClient){
			this->_IPClient = (std::string)pIPClient;
		}
		void									setPathInfo(void){
			this->_extension =  (this->_uri.find(".") != SIZE_MAX) ? &this->_uri[this->_uri.find(".")] : "";
			if (this->_extension.empty())
				this->_pathInfo = "";
			else
				this->_pathInfo = (this->_extension.find("/") != SIZE_MAX) ? &this->_extension[this->_extension.find("/") + 1] : "";
		}
		/***************************************************
		*******************    SEND   **********************
		***************************************************/
		void									sendPacket(std::string content){
			std::cout << RED << "On envoi a : " << this->_fd << RESET << std::endl;
			std::cout << RED << "Taille de l'envoi " << content.size() << RESET << std::endl;
			std::cout << RED << "L'envoi : " << content << RESET << std::endl;
			send(this->_fd, content.c_str(), content.size(), MSG_CONFIRM);
		}
		void									sendPacket(char *content, size_t len){
			std::cout << RED <<  "On envoi a : " << this->_fd << RESET << std::endl;
			std::cout << RED <<  "Taille de l'envoi " << len << RESET << std::endl;
			std::cout << RED <<  "L'envoi : " << content << RESET << std::endl;
			send(this->_fd, content, len, MSG_CONFIRM);
		}

		/***************************************************
		********************    FIND   *********************
		***************************************************/
		void									findUri(void){
			this->_uri = "";
			std::cout << GREEN << "avant uri : " << RESET << std::endl;
			this->_uri = this->_request.substr(this->_request.find("/"), (this->_request.find("HTTP") - 5));
			std::cout << GREEN << this->_uri << RESET << std::endl;
			this->_uri = cleanLine(this->_uri);
		}
		void									findTypeContent(void){
			this->_typeContent = "";
			this->_typeContent = this->_parsing->getMap()["Accept"];
		}


		/***************************************************
		******************    Parsing   ********************
		***************************************************/
		void									parsingMetasVars(void){
			this->_hostName = this->_parsing->getMap()["Host"].substr(0, this->_parsing->getMap()["Host"].find_first_of(":"));
			this->_hostPort = &this->_parsing->getMap()["Host"][this->_parsing->getMap()["Host"].find_first_of(":") + 1];
			this->_userAgent = this->_parsing->getMap()["User-Agent"];
		}
		void									parsingAuthorizations(void){
			std::string iss = this->_parsing->getMap()["Authorization"];
			iss = convertInSpaces(iss);
			iss = cleanLine(iss);
			std::vector<std::string> results = split(iss, " ");
			if (!results.empty()){
				this->_authType = results[0];
				this->_authCredentials = results[1];
			}
		}

private :
		int													_fd;

		std::string											_request;
		std::string											_uri;
		std::string											_typeContent;
		ParsingRequest *									_parsing;
		std::string											_method;
		std::string											_hostName;
		std::string											_hostPort;
		std::string											_IPClient;
		std::string											_userAgent;
		std::string											_authType;
		std::string											_authCredentials;
		std::string											_queryString;
		std::string 										_pathInfo;
		std::string											_extension;
		std::string											_datas;
		std::map<std::string, std::string> 					_mimesTypes;
};

#endif

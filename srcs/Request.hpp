#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "Header.hpp"

class Request {
	public:
		Request() {
			this->_socket = 0;
			this->_uri = NULL;
		}
		Request(int socket) {
			this->_socket = socket;
			this->_buffer[recv(this->_socket, &this->_buffer, sizeof(this->_buffer), 0)] = 0;
			this->_type = this->_buffer[0];
			std::string string(this->_buffer);
			this->_uri = strndup(&this->_buffer[string.find("/")], (string.find("H") - 5));
		}
		virtual ~Request() {
			close(this->_socket);
			return ; 
		}
		char			*get_uri() const {
			return (this->_uri);
		}
		char			*get_buffer(){
			return (this->_buffer);
		}
		char			get_type() const { 
			return (this->_type);
		}
		int				get_socket() const {
			return (this->_socket);
		}
		void			set_socket(int socket) {
			this->_socket = socket;
		}
		void			send_packet(const char *content) {
			send(this->_socket, content, strlen(content), 0);
		}

	private:
		int		_socket;
		char	_buffer[1025];
		char	*_uri;
		char	_type;
};

#endif
#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "ft_irc.hpp"

struct Away {
	bool status;
	std::string message;
};

class Client {
	private:
		int fd;
		std::string nickname;
		std::string username;
		std::string realname;
		std::string password;
		std::string temp_data;
		std::string hostname;
		std::string identifier;

		Server &server;
		bool authenticated;
		std::vector<Channel*> joined_channels;
		Away away;

	public:
		~Client();
		Client(const Client &copy);
		Client(int fd,  Server &server);
		Client &operator=(const Client &copy);

		void clean_tempData();
		bool read_into_tempData();
		bool buffer_has_linebreak();

		// getters:
		int get_fd() const;
		Server &getServer() const;
		const std::string &get_pass() const;
		const std::string &get_temp_data() const;
		const std::string &get_nickname() const;
		const std::string &get_username() const;
		const std::string &get_realname() const;
		const std::string &get_hostname() const;
		const std::string &get_identifier() const;
		const std::vector<Channel*> &get_joined_channels() const;
		const std::string &get_message() const;

		// setters:
		void add_channel(Channel* channel);
		void remove_channel(Channel* channel);
		void set_hostname(int client_socket);
		void set_authentication(bool status);
		void set_password(const std::string &password);
		void set_nickname(const std::string &nickname);
		void set_username(const std::string &username);
		void set_realname(const std::string &realname);
		void set_identifier(const std::string &identifier);
		void set_status(bool status);
		void set_message(const std::string &message);

		// checkers:
		bool is_authenticated() const;
		bool password_matched(Server &server) const;
		bool is_away() const;
};

#endif

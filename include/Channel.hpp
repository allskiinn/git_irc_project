#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include "ft_irc.hpp"

class Channel {
	private:
		std::string         	name_channel;       // nome do canal (começar com # ou &)
		std::string         	topic_channel;      // Topic do canal
		std::string         	pass_channel;       // Senha do canal (se o modo +k estiver ativo)
		std::deque<int> 		join_order; 		// Ordem de entrada dos membros
		std::map<int, Client*>	members_channel;    // Membros do canal (fd -> Client*)
		std::map<int, Client*>	invited_channel;    // Membros convidados para o canal
		std::map<int, Client*>	operators_channel;  // Operadores do canal (fd -> Client*)
		
		// Modos do canal
		bool byInviteOnly;      // mode +i
		bool topicRestricted; // mode +t
		bool passOn;          // mode +k
		size_t userLimit;     // mode +l

	public:
		~Channel();
		Channel(const Channel &copy);
		Channel& operator=(const Channel &copy);
		Channel(const std::string& name_channel, Client* new_channel);

		// Operações basicas do canal
		void addMember(Client* client);
		void removeMember(Client* client);
		void broadcast(Client* sender = NULL, const std::string& message = "");
		
		// Operações do operador de canal
		void addOperator(Client* client);
		void removeOperator(Client* client);
		void promoteFirstMember();
		bool isOperator(Client* client) const;
		bool isInvited(Client* client) const;
		int getCurrentMembersCount();
		int getCurrentOperatorsCount();
		int getUserLimit();
		std::string getModes() const;
		std::string getModeParams() const;
		
		void setInviteOnly(bool value);
		void setTopicRestricted(bool value);
		void setPassOn(const std::string& pass);
		void setUserLimit(size_t limit);
		
		bool mode(char mode) const;
		bool checkUserModes(char mode, Client* client);
		bool isMember(Client* client) const;
		
		// Getters
		const std::string& getName() const;
		const std::string& getTopic() const;
		const std::string& getPass() const;
		const std::map<int, Client*>& getMembers() const;
			
		// Exigido pelo subject
		void inviteMember(Client* target);
		void consumeInvite(Client* target);
		void setTopic(const std::string& new_topic);

		// Validações
		bool validChannelName(const std::string& name) const;
};

#endif

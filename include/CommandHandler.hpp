/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gkiala <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/11 14:11:08 by gkiala            #+#    #+#             */
/*   Updated: 2026/04/11 14:11:11 by gkiala           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMAND_HANDLER_HPP
#define COMMAND_HANDLER_HPP

#include "Server.hpp"
#include <string>
#include <vector>

class CommandHandler {
  private:
    CommandHandler();

    static std::vector<std::string> splitParams(const std::string &rest);
    static std::vector<std::string> splitByComma(const std::string &input);
    static std::string uppercase(const std::string &value);
    static std::string clientPrefix(const Client &client);

    static void cmdPass(Server &server, Client &client, const std::vector<std::string> &params);
    static void cmdNick(Server &server, Client &client, const std::vector<std::string> &params);
    static void cmdUser(Server &server, Client &client, const std::vector<std::string> &params);
    static void cmdPing(Server &server, Client &client, const std::vector<std::string> &params);
    static void cmdJoin(Server &server, Client &client, const std::vector<std::string> &params);
    static void cmdPart(Server &server, Client &client, const std::vector<std::string> &params);
    static void cmdPrivmsg(Server &server, Client &client, const std::vector<std::string> &params);
    static void cmdInvite(Server &server, Client &client, const std::vector<std::string> &params);
    static void cmdKick(Server &server, Client &client, const std::vector<std::string> &params);
    static void cmdTopic(Server &server, Client &client, const std::vector<std::string> &params);
    static void cmdMode(Server &server, Client &client, const std::vector<std::string> &params);
    static void cmdQuit(Server &server, Client &client, const std::vector<std::string> &params);

  public:
    static void handle(Server &server, Client &client, const std::string &line);
};

#endif


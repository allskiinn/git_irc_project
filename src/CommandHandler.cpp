/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gkiala <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/11 14:13:50 by gkiala            #+#    #+#             */
/*   Updated: 2026/04/13 00:00:00 by gkiala           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CommandHandler.hpp"
#include <cctype>
#include <cstdlib>
#include <sstream>

std::vector<std::string> CommandHandler::splitParams(const std::string &rest) {
  std::vector<std::string> params;
  std::istringstream iss(rest);
  std::string token;
  while (iss >> token) {
    if (!token.empty() && token[0] == ':') {
      std::string trailing = token.substr(1);
      std::string suffix;
      std::getline(iss, suffix);
      if (!suffix.empty() && suffix[0] == ' ') {
        suffix.erase(0, 1);
      }
      if (!suffix.empty()) {
        trailing += " " + suffix;
      }
      params.push_back(trailing);
      return params;
    }
    params.push_back(token);
  }
  return params;
}

std::vector<std::string> CommandHandler::splitByComma(const std::string &input) {
  std::vector<std::string> out;
  std::string part;
  std::istringstream iss(input);
  while (std::getline(iss, part, ',')) {
    out.push_back(part);
  }
  return out;
}

std::string CommandHandler::uppercase(const std::string &value) {
  std::string out = value;
  for (size_t i = 0; i < out.size(); ++i) {
    out[i] = static_cast<char>(std::toupper(out[i]));
  }
  return out;
}

std::string CommandHandler::clientPrefix(const Client &client) {
  return ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHost();
}

void CommandHandler::handle(Server &server, Client &client, const std::string &line) {
  std::istringstream iss(line);
  std::string command;
  iss >> command;
  if (command.empty()) {
    return;
  }

  std::string rest;
  std::getline(iss, rest);
  if (!rest.empty() && rest[0] == ' ') {
    rest.erase(0, 1);
  }
  std::vector<std::string> params = splitParams(rest);
  command = uppercase(command);

  if (command == "PASS") {
    cmdPass(server, client, params);
  } else if (command == "NICK") {
    cmdNick(server, client, params);
  } else if (command == "USER") {
    cmdUser(server, client, params);
  } else if (command == "PING") {
    cmdPing(server, client, params);
  } else if (command == "JOIN") {
    cmdJoin(server, client, params);
  } else if (command == "PART") {
    cmdPart(server, client, params);
  } else if (command == "PRIVMSG") {
    cmdPrivmsg(server, client, params);
  } else if (command == "INVITE") {
    cmdInvite(server, client, params);
  } else if (command == "KICK") {
    cmdKick(server, client, params);
  } else if (command == "TOPIC") {
    cmdTopic(server, client, params);
  } else if (command == "MODE") {
    cmdMode(server, client, params);
  } else if (command == "QUIT") {
    cmdQuit(server, client, params);
  } else {
    server.logAction("Unknown command: " + command);
    if (client.isRegistered()) {
      server.sendNumeric(client, 421, command + " :Unknown command");
    }
  }
}

void CommandHandler::cmdPass(Server &server, Client &client, const std::vector<std::string> &params) {
  if (params.empty()) {
    server.sendNumeric(client, 461, "PASS :Not enough parameters");
    return;
  }
  if (client.isRegistered()) {
    // FIX 462: texto conforme RFC 1459 — "You may not reregister"
    server.sendNumeric(client, 462, "You may not reregister");
    return;
  }
  if (params[0] != server.getPassword()) {
    server.sendNumeric(client, 464, "Password incorrect");
    return;
  }
  client.setPassAccepted(true);
  server.logAction("Password accepted");
  server.tryRegisterClient(client);
}

void CommandHandler::cmdNick(Server &server, Client &client, const std::vector<std::string> &params) {
  if (params.empty()) {
    server.sendNumeric(client, 431, "No nickname given");
    return;
  }
  if (!Server::isValidNickname(params[0])) {
    server.sendNumeric(client, 432, params[0] + " :Erroneous nickname");
    return;
  }
  if (server.isNicknameInUse(params[0], client.getFd())) {
    server.sendNumeric(client, 433, params[0] + " :Nickname is already in use");
    return;
  }
  const std::string oldNick = client.getNickname();
  client.setNickname(params[0]);

  if (!oldNick.empty() && oldNick != client.getNickname()) {
    std::string msg = ":" + oldNick + "!" + client.getUsername() + "@" + client.getHost();
    msg += " NICK :" + client.getNickname() + "\r\n";
    const std::set<std::string> &joined = client.getJoinedChannels();
    for (std::set<std::string>::const_iterator it = joined.begin(); it != joined.end(); ++it) {
      server.broadcastChannel(*it, msg, -1);
    }
  }

  server.tryRegisterClient(client);
}

void CommandHandler::cmdUser(Server &server, Client &client, const std::vector<std::string> &params) {
  std::string username;
  std::string realname;
  if (params.size() >= 4) {
    username = params[0];
    realname = params[3];
  } else if (params.size() == 3)
  {
    std::string::size_type colonPos = params[2].find(':');
    if (colonPos == std::string::npos) {
      server.sendNumeric(client, 461, "USER :Not enough parameters");
      return;
    }
    username = params[0];
    realname = params[2].substr(colonPos + 1);
  } else {
    server.sendNumeric(client, 461, "USER :Not enough parameters");
    return;
  }
  if (client.isRegistered()) {
    // FIX 462: texto conforme RFC 1459 — "You may not reregister"
    server.sendNumeric(client, 462, "You may not reregister");
    return;
  }
  client.setUsername(username);
  client.setRealname(realname);

  server.tryRegisterClient(client);
}

void CommandHandler::cmdPing(Server &server, Client &client, const std::vector<std::string> &params) {
  if (params.empty()) {
    server.sendNumeric(client, 409, "No origin specified");
    return;
  }
  server.sendToClient(client.getFd(), ":" + server.getServerName() + " PONG " + server.getServerName() + " :" + params[0] + "\r\n");
}

void CommandHandler::cmdJoin(Server &server, Client &client, const std::vector<std::string> &params) {
  if (!client.isRegistered()) {
    server.sendNumeric(client, 451, "You are not registered");
    return;
  }
  if (params.empty()) {
    server.sendNumeric(client, 461, "JOIN :Not enough parameters");
    return;
  }

  std::vector<std::string> channels = splitByComma(params[0]);
  std::vector<std::string> keys;
  if (params.size() > 1) {
    keys = splitByComma(params[1]);
  }

  for (size_t i = 0; i < channels.size(); ++i) {
    const std::string channelName = channels[i];
    const std::string key = (i < keys.size() ? keys[i] : "");
    if (channelName.empty() || channelName[0] != '#') {
      server.sendNumeric(client, 476, channelName + " :Invalid channel name");
      continue;
    }

    Channel &channel = server.getOrCreateChannel(channelName);
    const bool alreadyMember = channel.isMember(client.getFd());
    if (alreadyMember) {
      continue;
    }
    if (channel.isInviteOnly() && !channel.isInvited(client.getFd())) {
      server.sendNumeric(client, 473, channelName + " :Invite-only channel (+i)");
      continue;
    }
    if (!channel.getKey().empty() && channel.getKey() != key) {
      server.sendNumeric(client, 475, channelName + " :Bad channel key (+k)");
      continue;
    }
    if (channel.isFull()) {
      server.sendNumeric(client, 471, channelName + " :Channel is full (+l)");
      continue;
    }

    channel.addMember(client.getFd());
    client.joinChannel(channelName);
    channel.uninvite(client.getFd());
    if (channel.getOperators().empty()) {
      channel.addOperator(client.getFd());
    }
    server.logAction(client.getNickname() + " joined " + channelName);

    std::string joinMsg = clientPrefix(client) + " JOIN " + channelName + "\r\n";
    server.broadcastChannel(channelName, joinMsg, -1);

    if (channel.getTopic().empty()) {
      server.sendNumeric(client, 331, channelName + " :No topic is set");
    } else {
      server.sendNumeric(client, 332, channelName + " :" + channel.getTopic());
    }

    std::string names;
    const std::set<int> &members = channel.getMembers();
    for (std::set<int>::const_iterator it = members.begin(); it != members.end(); ++it) {
      const Client *memberClient = server.getClient(*it);
      if (memberClient == NULL) {
        continue;
      }
      if (!names.empty()) {
        names += " ";
      }
      if (channel.isOperator(*it)) {
        names += "@";
      }
      names += memberClient->getNickname();
    }
    server.sendToClient(client.getFd(),
                        ":" + server.getServerName() + " 353 " + client.getNickname() + " = " +
                            channelName + " :" + names + "\r\n");
    server.sendNumeric(client, 366, channelName + " :End of /NAMES list");
  }
}

void CommandHandler::cmdPart(Server &server, Client &client, const std::vector<std::string> &params) {
  if (!client.isRegistered()) {
    server.sendNumeric(client, 451, "You are not registered");
    return;
  }
  if (params.empty()) {
    server.sendNumeric(client, 461, "PART :Not enough parameters");
    return;
  }
  std::vector<std::string> channels = splitByComma(params[0]);
  for (size_t i = 0; i < channels.size(); ++i) {
    Channel *channel = server.getChannel(channels[i]);
    if (channel == NULL) {
      server.sendNumeric(client, 403, channels[i] + " :No such channel");
      continue;
    }
    if (!channel->isMember(client.getFd())) {
      server.sendNumeric(client, 442, channels[i] + " :You're not on that channel");
      continue;
    }
    std::string partMsg = clientPrefix(client) + " PART " + channels[i] + "\r\n";
    server.broadcastChannel(channels[i], partMsg, -1);
    channel->removeMember(client.getFd());
    client.leaveChannel(channels[i]);
    server.logAction(client.getNickname() + " left " + channels[i]);
    server.eraseChannelIfEmpty(channels[i]);
  }
}

void CommandHandler::cmdPrivmsg(Server &server, Client &client, const std::vector<std::string> &params) {
  if (!client.isRegistered()) {
    server.sendNumeric(client, 451, "You are not registered");
    return;
  }
  if (params.size() < 2) {
    server.sendNumeric(client, 461, "PRIVMSG :Not enough parameters");
    return;
  }

  const std::string &target = params[0];
  const std::string &text = params[1];
  if (target.empty()) {
    server.sendNumeric(client, 411, "No recipient given (PRIVMSG)");
    return;
  }
  if (text.empty()) {
    server.sendNumeric(client, 412, "No text to send");
    return;
  }

  std::string msg = clientPrefix(client) + " PRIVMSG " + target + " :" + text + "\r\n";
  if (!target.empty() && target[0] == '#') {
    Channel *channel = server.getChannel(target);
    if (channel == NULL) {
      server.sendNumeric(client, 403, target + " :No such channel");
      return;
    }
    if (!channel->isMember(client.getFd())) {
      server.sendNumeric(client, 404, target + " :Cannot send to channel");
      return;
    }
    server.broadcastChannel(target, msg, client.getFd());
    server.logAction(client.getNickname() + " sent a message to " + target);
    return;
  }

  Client *dst = server.getClientByNick(target);
  if (dst == NULL) {
    server.sendNumeric(client, 401, target + " :No such nick");
    return;
  }
  server.sendToClient(dst->getFd(), msg);
  server.logAction(client.getNickname() + " sent a private message to " + dst->getNickname());
  if (dst->isAway()) {
    server.sendNumeric(client, 301, dst->getNickname() + " :" + dst->getAwayMessage());
  }
}

void CommandHandler::cmdInvite(Server &server, Client &client, const std::vector<std::string> &params) {
  if (!client.isRegistered()) {
    server.sendNumeric(client, 451, "You are not registered");
    return;
  }
  if (params.size() < 2) {
    server.sendNumeric(client, 461, "INVITE :Not enough parameters");
    return;
  }

  Client *target = server.getClientByNick(params[0]);
  if (target == NULL) {
    server.sendNumeric(client, 401, params[0] + " :No such nick");
    return;
  }
  Channel *channel = server.getChannel(params[1]);
  if (channel == NULL) {
    server.sendNumeric(client, 403, params[1] + " :No such channel");
    return;
  }
  if (!channel->isMember(client.getFd())) {
    server.sendNumeric(client, 442, params[1] + " :You're not on that channel");
    return;
  }
  if (!channel->isOperator(client.getFd())) {
    server.sendNumeric(client, 482, params[1] + " :You're not channel operator");
    return;
  }
  if (channel->isMember(target->getFd())) {
    server.sendNumeric(client, 443, target->getNickname() + " " + params[1] + " :is already on channel");
    return;
  }

  channel->invite(target->getFd());
  server.logAction(client.getNickname() + " invited " + target->getNickname() + " to " + params[1]);
  server.sendNumeric(client, 341, target->getNickname() + " " + params[1]);
  std::string inviteMsg = clientPrefix(client) + " INVITE " + target->getNickname() + " :" + params[1] + "\r\n";
  server.sendToClient(target->getFd(), inviteMsg);
}

void CommandHandler::cmdKick(Server &server, Client &client, const std::vector<std::string> &params) {
  if (!client.isRegistered()) {
    server.sendNumeric(client, 451, "You are not registered");
    return;
  }
  if (params.size() < 2) {
    server.sendNumeric(client, 461, "KICK :Not enough parameters");
    return;
  }

  Channel *channel = server.getChannel(params[0]);
  if (channel == NULL) {
    server.sendNumeric(client, 403, params[0] + " :No such channel");
    return;
  }
  if (!channel->isOperator(client.getFd())) {
    server.sendNumeric(client, 482, params[0] + " :You're not channel operator");
    return;
  }

  // FIX 441: verificar se o nick está no canal ANTES de procurar globalmente.
  // Se o nick não existir no canal (seja porque não existe de todo, ou porque
  // simplesmente não está lá), o erro correto é 441 ERR_USERNOTINCHANNEL.
  // Só precisamos do objeto Client para executar o kick — não para validar.
  Client *target = server.getClientByNick(params[1]);
  if (target == NULL || !channel->isMember(target->getFd())) {
    server.sendNumeric(client, 441, params[1] + " " + params[0] + " :They aren't on that channel");
    return;
  }

  std::string reason = (params.size() >= 3 ? params[2] : client.getNickname());
  std::string kickMsg = clientPrefix(client) + " KICK " + params[0] + " " + params[1] + " :" + reason + "\r\n";
  server.broadcastChannel(params[0], kickMsg, -1);
  channel->removeMember(target->getFd());
  target->leaveChannel(params[0]);
  server.logAction(client.getNickname() + " kicked " + target->getNickname() + " from " + params[0]);
  server.eraseChannelIfEmpty(params[0]);
}

void CommandHandler::cmdTopic(Server &server, Client &client, const std::vector<std::string> &params) {
  if (!client.isRegistered()) {
    server.sendNumeric(client, 451, "You are not registered");
    return;
  }
  if (params.empty()) {
    server.sendNumeric(client, 461, "TOPIC :Not enough parameters");
    return;
  }
  Channel *channel = server.getChannel(params[0]);
  if (channel == NULL) {
    server.sendNumeric(client, 403, params[0] + " :No such channel");
    return;
  }
  if (!channel->isMember(client.getFd())) {
    server.sendNumeric(client, 442, params[0] + " :You're not on that channel");
    return;
  }
  if (params.size() == 1) {
    if (channel->getTopic().empty()) {
      server.sendNumeric(client, 331, params[0] + " :No topic is set");
    } else {
      server.sendNumeric(client, 332, params[0] + " :" + channel->getTopic());
    }
    return;
  }
  if (channel->isTopicRestricted() && !channel->isOperator(client.getFd())) {
    server.sendNumeric(client, 482, params[0] + " :You're not channel operator");
    return;
  }
  channel->setTopic(params[1]);
  server.logAction(client.getNickname() + " changed topic on " + params[0]);
  server.broadcastChannel(params[0], clientPrefix(client) + " TOPIC " + params[0] + " :" + params[1] + "\r\n", -1);
}

void CommandHandler::cmdMode(Server &server, Client &client, const std::vector<std::string> &params) {
  if (!client.isRegistered()) {
    server.sendNumeric(client, 451, "You are not registered");
    return;
  }
  if (params.empty()) {
    server.sendNumeric(client, 461, "MODE :Not enough parameters");
    return;
  }

  // FIX 221: se o target não começar por '#', é um user MODE.
  // Responde com 221 RPL_UMODEIS com os modos do utilizador (simplificado: "+").
  if (params[0].empty() || params[0][0] != '#') {
    // Verifica se o nick pedido é o próprio (único caso permitido no ft_irc)
    if (params[0] == client.getNickname()) {
      server.sendNumeric(client, 221, "+");
    } else {
      server.sendNumeric(client, 502, "Cannot change mode for other users");
    }
    return;
  }

  Channel *channel = server.getChannel(params[0]);
  if (channel == NULL) {
    server.sendNumeric(client, 403, params[0] + " :No such channel");
    return;
  }
  if (params.size() == 1) {
    std::string currentModes = "+";
    std::string modeArgs;
    if (channel->isInviteOnly()) {
      currentModes += "i";
    }
    if (channel->isTopicRestricted()) {
      currentModes += "t";
    }
    if (!channel->getKey().empty()) {
      currentModes += "k";
      modeArgs += " " + channel->getKey();
    }
    if (channel->getUserLimit() > 0) {
      std::ostringstream oss;
      oss << channel->getUserLimit();
      currentModes += "l";
      modeArgs += " " + oss.str();
    }
    server.sendNumeric(client, 324, params[0] + " " + currentModes + modeArgs);
    return;
  }
  if (!channel->isOperator(client.getFd())) {
    server.sendNumeric(client, 482, params[0] + " :You're not channel operator");
    return;
  }

  bool add = true;
  size_t argIndex = 2;
  std::string changedModes;
  std::string changedArgs;
  const std::string flags = params[1];

  for (size_t i = 0; i < flags.size(); ++i) {
    const char flag = flags[i];
    if (flag == '+') {
      add = true;
      changedModes += "+";
      continue;
    }
    if (flag == '-') {
      add = false;
      changedModes += "-";
      continue;
    }

    if (flag == 'i') {
      channel->setInviteOnly(add);
      changedModes += "i";
    } else if (flag == 't') {
      channel->setTopicRestricted(add);
      changedModes += "t";
    } else if (flag == 'k') {
      if (add) {
        if (argIndex >= params.size()) {
          continue;
        }
        channel->setKey(params[argIndex++]);
        changedModes += "k";
        changedArgs += " " + channel->getKey();
      } else {
        channel->setKey("");
        changedModes += "k";
      }
    } else if (flag == 'o') {
      if (argIndex >= params.size()) {
        continue;
      }
      Client *target = server.getClientByNick(params[argIndex++]);
      if (target == NULL || !channel->isMember(target->getFd())) {
        continue;
      }
      if (add) {
        channel->addOperator(target->getFd());
      } else {
        channel->removeOperator(target->getFd());
      }
      changedModes += "o";
      changedArgs += " " + target->getNickname();
    } else if (flag == 'l') {
      if (add) {
        if (argIndex >= params.size()) {
          continue;
        }
        int limit = std::atoi(params[argIndex++].c_str());
        if (limit > 0) {
          channel->setUserLimit(limit);
          changedModes += "l";
          std::ostringstream oss;
          oss << limit;
          changedArgs += " " + oss.str();
        }
      } else {
        channel->setUserLimit(0);
        changedModes += "l";
      }
    } else {
      server.sendNumeric(client, 472, std::string(1, flag) + " :Unknown mode");
    }
  }

  if (!changedModes.empty()) {
    std::string modeMsg = clientPrefix(client) + " MODE " + params[0] + " " + changedModes + changedArgs + "\r\n";
    server.broadcastChannel(params[0], modeMsg, -1);
    server.logAction(client.getNickname() + " changed mode on " + params[0] + " to " + changedModes + changedArgs);
  }
}

void CommandHandler::cmdQuit(Server &server, Client &client, const std::vector<std::string> &params) {
  std::string reason = "Client Quit";
  if (!params.empty()) {
    reason = params[0];
  }
  server.sendToClient(client.getFd(), "ERROR :Closing Link: " + reason + "\r\n");
  server.disconnectClient(client.getFd(), reason);
}
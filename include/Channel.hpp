/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gkiala <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/11 14:10:46 by gkiala            #+#    #+#             */
/*   Updated: 2026/04/11 14:10:47 by gkiala           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <set>
#include <string>

class Channel {
  private:
    std::string _name;
    std::string _topic;
    std::string _key;
    bool _inviteOnly;
    bool _topicRestricted;
    int _userLimit;
    std::set<int> _members;
    std::set<int> _operators;
    std::set<int> _invited;

  public:
    Channel();
    Channel(const std::string &name);

    const std::string &getName() const;
    const std::string &getTopic() const;
    const std::string &getKey() const;
    bool isInviteOnly() const;
    bool isTopicRestricted() const;
    int getUserLimit() const;
    const std::set<int> &getMembers() const;
    const std::set<int> &getOperators() const;

    bool isMember(int fd) const;
    bool isOperator(int fd) const;
    bool isInvited(int fd) const;
    bool isFull() const;

    void setTopic(const std::string &topic);
    void setKey(const std::string &key);
    void setInviteOnly(bool state);
    void setTopicRestricted(bool state);
    void setUserLimit(int limit);

    void addMember(int fd);
    void removeMember(int fd);
    void addOperator(int fd);
    void removeOperator(int fd);
    void invite(int fd);
    void uninvite(int fd);
    bool empty() const;
};

#endif


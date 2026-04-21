/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gkiala <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/11 14:11:41 by gkiala            #+#    #+#             */
/*   Updated: 2026/04/11 14:11:52 by gkiala           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <cstdlib>
#include <exception>
#include <iostream>

int main(int ac, char **av) {
  if (ac != 3) {
    std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
    return 1;
  }

  int port = std::atoi(av[1]);
  if (port <= 0 || port > 65535) {
    std::cerr << "Error: invalid port" << std::endl;
    return 1;
  }
  if (std::string(av[2]).empty()) {
    std::cerr << "Error: password cannot be empty" << std::endl;
    return 1;
  }

  try {
    Server server(port, av[2]);
    server.start();
  } catch (const std::exception &e) {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}

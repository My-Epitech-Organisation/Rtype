/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Option - Option and positional argument definitions
*/

#ifndef SRC_COMMON_ARGPARSER_OPTION_HPP_
#define SRC_COMMON_ARGPARSER_OPTION_HPP_

#include <string>

namespace rtype {

/**
 * @brief Option definition for argument parser
 */
struct Option {
    std::string shortOpt;       ///< Short option (e.g., "-h")
    std::string longOpt;        ///< Long option (e.g., "--help")
    std::string description;    ///< Description for help message
    bool hasArg = false;        ///< Whether option takes an argument
    std::string argName;        ///< Name of the argument (for help message)
};

/**
 * @brief Positional argument definition
 */
struct PositionalArg {
    std::string name;           ///< Name of the argument (for help message)
    std::string description;    ///< Description for help message
    bool required = true;       ///< Whether this argument is required
};

}  // namespace rtype

#endif  // SRC_COMMON_ARGPARSER_OPTION_HPP_

/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ParseResult - Argument parsing result enumeration
*/

#ifndef SRC_COMMON_ARGPARSER_PARSERESULT_HPP_
#define SRC_COMMON_ARGPARSER_PARSERESULT_HPP_

namespace rtype {

/**
 * @brief Result of argument parsing
 */
enum class ParseResult {
    Success,  ///< Parsing succeeded, continue execution
    Exit,     ///< Parsing succeeded but should exit (e.g., --help)
    Error     ///< Parsing failed due to an error
};

}  // namespace rtype

#endif  // SRC_COMMON_ARGPARSER_PARSERESULT_HPP_

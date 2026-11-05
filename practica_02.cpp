#include <iostream>
#include <stdexcept>
#include <string>

#include <llparser.hpp>

using namespace Designar;

Grammar make_grammar()
{
    Grammar g;
    g.set_start_symbol("E");

    g.add_production("E", Grammar::Sequence({"T", "E'"}));
    g.add_production("E'", Grammar::Sequence({"PLUS", "T", "E'"}));
    g.add_production("E'", Grammar::Sequence({"MINUS", "T", "E'"}));
    g.add_production("E'", Grammar::Sequence());

    g.add_production("T", Grammar::Sequence({"F", "T'"}));
    g.add_production("T'", Grammar::Sequence({"TIMES", "F", "T'"}));
    g.add_production("T'", Grammar::Sequence());

    g.add_production("F",
                     Grammar::Sequence({"LPAREN", "E", "RPAREN"}));
    g.add_production("F", Grammar::Sequence({"NUM"}));
    return g;
}

Lexer make_lexer()
{
    Lexer lexer;
    lexer.add_token("NUM",
                    "(0|1|2|3|4|5|6|7|8|9)"
                    "(0|1|2|3|4|5|6|7|8|9)*");
    lexer.add_token("PLUS", "\\+");
    lexer.add_token("MINUS", "-");
    lexer.add_token("TIMES", "\\*");
    lexer.add_token("LPAREN", "\\(");
    lexer.add_token("RPAREN", "\\)");
    lexer.add_token("WS", " +");
    return lexer;
}

DynArray<Token> tokenize_without_spaces(const Lexer& lexer,
                                        const std::string& input)
{
    DynArray<Token> all = lexer.tokenize(input);
    DynArray<Token> tokens;

    for (nat_t i = 0; i < all.size(); ++i)
    {
        if (all[i].name != "WS")
        {
            tokens.append(all[i]);
        }
    }

    return tokens;
}

void print_tree(ParseTreeNode* node, nat_t depth = 0)
{
    for (nat_t i = 0; i < depth; ++i)
    {
        std::cout << "  ";
    }

    std::cout << node->get_key().symbol;
    if (!node->get_key().lexeme.empty())
    {
        std::cout << " (\"" << node->get_key().lexeme << "\")";
    }
    std::cout << '\n';

    ParseTreeNode* child = node->get_first_child();
    while (child != nullptr)
    {
        print_tree(child, depth + 1);
        child = child->get_right_sibling();
    }
}

int main()
{
    Grammar grammar = make_grammar();
    LLParser parser(grammar);
    Lexer lexer = make_lexer();

    const std::string input = "(9-*2";

    try
    {
        DynArray<Token> tokens = tokenize_without_spaces(lexer, input);
        ParseTreeNode* tree = parser.parse(tokens);

        std::cout << "Entrada: " << input << '\n';
        std::cout << "Resultado: Aceptada\n\n";
        std::cout << "Arbol de analisis:\n";
        print_tree(tree);
        ParseTreeNode::destroy_tree(tree);
    }
    catch (const std::runtime_error& error)
    {
        std::cout << "Entrada: " << input << '\n';
        std::cout << "Resultado: Rechazada\n";
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}


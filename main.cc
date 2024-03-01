// Game of Othello -- Example of main
// Universidad Simon Bolivar, 2012.
// Author: Blai Bonet
// Last Revision: 1/11/16
// Modified by:

#include <iostream>
#include <limits>
#include "othello_cut.h" // won't work correctly until .h is fixed!
#include "utils.h"

#include <unordered_map>

using namespace std;

unsigned expanded = 0;
unsigned generated = 0;
int tt_threshold = 32; // threshold to save entries in TT

// Transposition table (it is not necessary to implement TT)
struct stored_info_t
{
    int value_;
    int type_;
    enum
    {
        EXACT,
        LOWER,
        UPPER
    };
    stored_info_t(int value = -100, int type = LOWER) : value_(value), type_(type) {}
};

struct hash_function_t
{
    size_t operator()(const state_t &state) const
    {
        return state.hash();
    }
};

class hash_table_t : public unordered_map<state_t, stored_info_t, hash_function_t>
{
};

hash_table_t TTable[2];

// int maxmin(state_t state, int depth, bool use_tt);
// int minmax(state_t state, int depth, bool use_tt = false);
// int maxmin(state_t state, int depth, bool use_tt = false);
int negamax(state_t state, int depth, int color, bool use_tt = false)
{
    // si la profundidad == 0 o es un estado terminal, retornamos el valor del estado
    if (depth == 0 || state.terminal())
        return color * state.value();

    // si no, el score lo inicializamos en -infinito
    int score = std::numeric_limits<int>::min();
    // generamos los estados para cada movimiento valido y los guardamos en un vector
    std::vector<state_t> valid_states = state.get_valid_moves(color == 1);
    // iterar en cada elemento del vector
    for (auto st : valid_states)
    {
        // aumentamos la cantidad de nodos generados
        generated++;
        // llamamos recursivamente a negamax
        score = max(score, -negamax(st, depth - 1, -color));
    }
    // aumentar la cantidad de nodos expandidos
    expanded++;
    // retornar el score
    return score;
}

int minimax(state_t state, int depth, int alpha, int beta, int color){
    int score;
    // Si la profundidad es 0 o es un estado terminal, retornamos el valor del estado
    if (depth == 0 || state.terminal())
        return state.value();
    
    // Si el estado es de max, entonces
    if (color == 1){
        std::vector<state_t> valid_states = state.get_valid_moves(true);
        // Iteramos en cada estado valido
        for (auto st : valid_states){
            generated++;
            // Actualizamos el score con el maximo entre el score y el valor de minimax recursivo
            score = max(score, minimax(st, depth - 1, alpha, beta, -color));
            // Actualizamos el valor de alpha
            alpha = max(alpha, score);
            // Si beta es menor o igual a alpha, podamos
            if (beta <= alpha) // Poda alfa
                break;
        }
        expanded++;
        return alpha;
    }
    // Si el estado es de min, entonces
    else {
        std::vector<state_t> valid_states = state.get_valid_moves(false);
        // Iteramos en cada estado valido
        for (auto st : valid_states){
            generated++;
            // Actualizamos el score con el minimo entre el score y el valor de minimax recursivo
            score = min(score, minimax(st, depth - 1, alpha, beta, -color));
            // Actualizamos el valor de beta
            beta = min(beta, score);
            // Si beta es menor o igual a alpha, podamos
            if (beta <= alpha) // Poda beta
                break;
        }
        expanded++;
        return beta;
    }

}
int negamax(state_t state, int depth, int alpha, int beta, int color, bool use_tt = false){
    // Si la profundidad es 0 o es un estado terminal, retornamos el valor del estado
    if (depth == 0 || state.terminal())
        return color * state.value();
    // Si no, el score lo inicializamos en -infinito
    int score = std::numeric_limits<int>::min();
    // Iteramos en cada estado valido
    std::vector<state_t> valid_states = state.get_valid_moves(color == 1);
    for (auto st : valid_states){
        generated++;
        // Llamamos recursivamente a negamax
        int val = -negamax(st, depth - 1, -beta, -alpha, -color);
        // Actualizamos el score con el maximo entre el score y el valor de negamax recursivo
        score = max(score, val);
        // Actualizamos el valor de alpha
        alpha = max(alpha, val);
        // Si beta es menor o igual a alpha, podamos
        if (alpha >= beta)
            break;
    }
    expanded++;
    return score;
}
int scout(state_t state, int depth, int color, bool use_tt = false);
int negascout(state_t state, int depth, int alpha, int beta, int color, bool use_tt = false);

int main(int argc, const char **argv)
{
    state_t pv[128];
    int npv = 0;
    for (int i = 0; PV[i] != -1; ++i)
        ++npv;

    int algorithm = 0;
    if (argc > 1)
        algorithm = atoi(argv[1]);
    bool use_tt = argc > 2;

    // Extract principal variation of the game
    state_t state;
    cout << "Extracting principal variation (PV) with " << npv << " plays ... " << flush;
    for (int i = 0; PV[i] != -1; ++i)
    {
        bool player = i % 2 == 0; // black moves first!
        int pos = PV[i];
        pv[npv - i] = state;
        state = state.move(player, pos);
    }
    pv[0] = state;
    cout << "done!" << endl;

#if 0
    // print principal variation
    for( int i = 0; i <= npv; ++i )
        cout << pv[npv - i];
#endif

    // Print name of algorithm
    cout << "Algorithm: ";
    if (algorithm == 1)
        cout << "Negamax (minmax version)";
    else if (algorithm == 2)
        cout << "Negamax (alpha-beta version)";
    else if (algorithm == 3)
        cout << "Scout";
    else if (algorithm == 4)
        cout << "Negascout";
    cout << (use_tt ? " w/ transposition table" : "") << endl;

    // Run algorithm along PV (bacwards)
    cout << "Moving along PV:" << endl;
    for (int i = 0; i <= npv; ++i)
    {
        // cout << pv[i];
        int value = 0;
        TTable[0].clear();
        TTable[1].clear();
        float start_time = Utils::read_time_in_seconds();
        expanded = 0;
        generated = 0;
        int color = i % 2 == 1 ? 1 : -1;

        try
        {
            if (algorithm == 1)
            {
                // value = negamax(pv[i], 0, color, use_tt);
            }
            else if (algorithm == 2)
            {
                // value = negamax(pv[i], 0, -200, 200, color, use_tt);
            }
            else if (algorithm == 3)
            {
                // value = scout(pv[i], 0, color, use_tt);
            }
            else if (algorithm == 4)
            {
                // value = negascout(pv[i], 0, -200, 200, color, use_tt);
            }
        }
        catch (const bad_alloc &e)
        {
            cout << "size TT[0]: size=" << TTable[0].size() << ", #buckets=" << TTable[0].bucket_count() << endl;
            cout << "size TT[1]: size=" << TTable[1].size() << ", #buckets=" << TTable[1].bucket_count() << endl;
            use_tt = false;
        }

        float elapsed_time = Utils::read_time_in_seconds() - start_time;

        cout << npv + 1 - i << ". " << (color == 1 ? "Black" : "White") << " moves: "
             << "value=" << color * value
             << ", #expanded=" << expanded
             << ", #generated=" << generated
             << ", seconds=" << elapsed_time
             << ", #generated/second=" << generated / elapsed_time
             << endl;
    }

    return 0;
}

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

enum Condicion {MAYOR_IGUAL, MAYOR};

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


int negamax(state_t state, int depth, int alpha, int beta, int color, bool use_tt = false) {
    // condicion base = profundidad alcanzada o estado terminal
    if (depth == 0 || state.terminal())
        return color * state.value(); // Valor del estado terminal multiplicado por el color

    int score = std::numeric_limits<int>::min(); // se inicializa el puntaje con el valor minimo de int
    std::vector<state_t> moves = state.get_valid_moves(color == 1); // obtener movimientos validos

    // iterar sobre los movimientos
    for (const auto& nextState : moves) {
        generated++; // se incrementar contador de estados generados

        // calcular el valor del siguiente estado recursivamente y actualiza el puntaje maximo
        int val = -negamax(nextState, depth - 1, -beta, -alpha, -color);
        score = std::max(score, val);
        alpha = std::max(alpha, val);

        // realizar la poda beta
        if (alpha >= beta)
            break;
    }

    expanded++;
    return score;
}


bool TEST(state_t state, int depth, int color, int score, Condicion cond) {
    // condicion base = profundidad alcanzada o estado terminal
    if (depth == 0 || state.terminal()) 
        return state.value() > score;

    // determinar si es una maximizacion o minimizacion
    bool isMax = color == 1;

    // obtener movimientos validos
    std::vector<state_t> moves = state.get_valid_moves(isMax);

    // iterar sobre los movimientos
    for (const auto& nextState : moves) {
        generated++;

        // realizar la prueba recursiva segun la condicion
        bool testResult;
        if (isMax && cond == Condicion::MAYOR)
            testResult = TEST(nextState, depth - 1, -color, score, Condicion::MAYOR);
        else if (!isMax && cond == Condicion::MAYOR)
            testResult = !TEST(nextState, depth - 1, -color, score, Condicion::MAYOR);
        else // cond == Condicion::MAYOR_IGUAL
            testResult = TEST(nextState, depth - 1, -color, score, Condicion::MAYOR) || state.value() >= score;

        // si se cumple la condicion, devolver el resultado
        if (testResult == (cond == Condicion::MAYOR))
            return true;
    }

    expanded++;
    return !isMax;
}


int scout(state_t state, int depth, int color, bool use_tt = false) {
    // condicion base = profundidad alcanzada o estado terminal
    if (depth == 0 || state.terminal()) {
        return state.value(); // valor del estado terminal
    }
    
    bool isMax = color == 1; // determinar si es una maximizacion o minimizacion
    int score = 0;

    // obtener movimientos validos
    std::vector<state_t> moves = state.get_valid_moves(isMax);

    // iterar sobre los movimientos
    for (auto& nextState : moves) {
        generated++; // incrementar contador de estados generados

        // si es el primer movimiento realiza una busqueda scout
        if (score == 0) {
            score = scout(nextState, depth - 1, -color); 
            continue;
        }

        // evalua el siguiente movimiento
        bool testResult = isMax ? TEST(nextState, depth, -color, score, Condicion::MAYOR)
                                : !TEST(nextState, depth, -color, score, Condicion::MAYOR_IGUAL);

        // si el movimiento cumple la condicion de corte realiza una busqueda scout
        if (testResult == (isMax && testResult)) { 
            score = scout(nextState, depth - 1, -color);
        }
    }

    expanded++;
    return score;
}


int negascout(state_t state, int depth, int alpha, int beta, int color, bool use_tt = false) {
    // condicion base = profundidad alcanzada o estado terminal
    if (depth == 0 || state.terminal()) 
        return color * state.value();

    int score = 0;
    bool first = true;

    // obtener movimientos validos
    std::vector<state_t> moves = state.get_valid_moves(color == 1);

    // iterar sobre los movimientos
    for (auto& nextState : moves) {
        generated++;

        if (first) {
            // realiza la primera busqueda recursiva con alpha y beta invertidos
            score = -negascout(nextState, depth - 1, -beta, -alpha, -color);
            first = false;
        } else {
            // realiza una busqueda scout con ventana estrecha
            score = -negascout(nextState, depth - 1, -alpha - 1, -alpha, -color);
            // si la busqueda scout no fue exitosa se realiza una nueva busqueda con ventana completa
            if (alpha < score && score < beta) {
                score = -negascout(nextState, depth - 1, -beta, -score, -color);
            }
        }

        // actualiza el valor de alpha
        alpha = std::max(alpha, score);

        // realizar la poda beta
        if (alpha >= beta) 
            break;
    }

    expanded++;
    return alpha;
}


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
    for( int i = 0; i <= npv; ++i ){
      cout << pv[npv - i] << endl;
      pv[npv - i].print_bits(cout);
      cout << endl;
    }  
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
                value = negamax(pv[i], 20, color, use_tt);
            }
            else if (algorithm == 2)
            {
                value = negamax(pv[i], 20, -200, 200, color, use_tt);
            }
            else if (algorithm == 3)
            {
                value = scout(pv[i], 20, color, use_tt);
            }
            else if (algorithm == 4)
            {
                value = negascout(pv[i], 20, -200, 200, color, use_tt);
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

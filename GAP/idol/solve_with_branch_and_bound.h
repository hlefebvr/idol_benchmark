//
// Created by henri on 29/11/22.
//

#ifndef IDOL_BENCHMARK_SOLVE_WITH_BRANCH_AND_BOUND_H
#define IDOL_BENCHMARK_SOLVE_WITH_BRANCH_AND_BOUND_H

#include "write_to_file.h"
#include "modeling.h"
#include "algorithms.h"
#include "problems/GAP/GAP_Instance.h"
#include "algorithms/callbacks/Callbacks_RoundingHeuristic.h"

using namespace Problems::GAP;

void solve_with_branch_and_bound(const std::string& t_path_to_instance) {

    const auto instance = read_instance(t_path_to_instance);

    const unsigned int n_knapsacks = instance.n_knapsacks();
    const unsigned int n_items = instance.n_items();

    Model model;

    auto x = model.add_vars(Dim<2>(n_knapsacks, n_items), 0., 1., Continuous, 0.);

    for (unsigned int k = 0 ; k < n_knapsacks ; ++k) {
        for (unsigned int j = 0 ; j < n_items ; ++j) {
            model.set(Attr::Var::Obj, x[k][j], instance.p(k, j));
        }

        model.add_ctr(idol_Sum(j, Range(n_items), instance.w(k,j) * x[k][j]) <= instance.t(k));
    }

    for (unsigned int j = 0 ; j < n_items ; ++j) {
        model.add_ctr(idol_Sum(k, Range(n_knapsacks), x[k][j]) == 1);
    }

    std::vector<Var> branching_candidates;
    branching_candidates.reserve(n_knapsacks * n_items);
    for (unsigned int k = 0 ; k < n_knapsacks ; ++k) {
        for (unsigned int j = 0; j < n_items; ++j) {
            branching_candidates.emplace_back(x[k][j]);
        }
    }

    auto solver = branch_and_bound<Solvers::GLPK>(model, branching_candidates);
    solver.add_callback<Callbacks::RoundingHeuristic>(branching_candidates);
    solver.set(Param::Algorithm::TimeLimit, TIME_LIMIT);
    solver.solve();

    write_to_file(
            t_path_to_instance,
            "idol_bab",
            n_knapsacks,
            n_items,
            solver.primal_solution().objective_value(),
            solver.time().count()
    );
}

#endif //IDOL_BENCHMARK_SOLVE_WITH_BRANCH_AND_BOUND_H
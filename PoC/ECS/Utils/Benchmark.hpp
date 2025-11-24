/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Benchmark - Performance measurement utilities
*/

#ifndef ECS_UTILS_BENCHMARK_HPP
    #define ECS_UTILS_BENCHMARK_HPP
    #include <chrono>
    #include <string>
    #include <vector>
    #include <functional>
    #include <iostream>
    #include <iomanip>

namespace ECS {

    /**
     * @brief Performance measurement and comparison tool.
     *
     * Features:
     * - Microsecond precision timing
     * - Multiple iterations for averaging
     * - Comparative benchmarks
     * - Formatted output
     *
     * Example:
     *   Benchmark bench;
     *   bench.measure("Test", []() { });
     *   bench.print_results();
     */
    class Benchmark {
    public:
        struct Result {
            std::string name;
            double avg_time_us;
            double min_time_us;
            double max_time_us;
            size_t iterations;
        };

        /**
         * @brief Measures execution time of a function.
         * @param name Test identifier
         * @param func Function to benchmark
         * @param iterations Number of runs for averaging
         */
        template<typename Func>
        void measure(const std::string& name, Func&& func, size_t iterations = 100) {
            std::vector<double> times;
            times.reserve(iterations);

            for (size_t i = 0; i < iterations; ++i) {
                auto start = std::chrono::high_resolution_clock::now();
                func();
                auto end = std::chrono::high_resolution_clock::now();
                double elapsed_us = std::chrono::duration<double, std::micro>(end - start).count();
                times.push_back(elapsed_us);
            }

            double sum = 0.0;
            double min_val = times[0];
            double max_val = times[0];

            for (double t : times) {
                sum += t;
                if (t < min_val) min_val = t;
                if (t > max_val) max_val = t;
            }

            _results.push_back({name, sum / iterations, min_val, max_val, iterations});
        }

        /**
         * @brief Prints all benchmark results in a formatted table.
         */
        void print_results() const {
            std::cout << "\n=== BENCHMARK RESULTS ===\n";
            std::cout << std::left << std::setw(30) << "Test Name"
                     << std::right << std::setw(15) << "Avg (μs)"
                     << std::setw(15) << "Min (μs)"
                     << std::setw(15) << "Max (μs)"
                     << std::setw(12) << "Iterations" << "\n";
            std::cout << std::string(87, '-') << "\n";

            for (const auto& result : _results) {
                std::cout << std::left << std::setw(30) << result.name
                         << std::right << std::setw(15) << std::fixed << std::setprecision(2) << result.avg_time_us
                         << std::setw(15) << result.min_time_us
                         << std::setw(15) << result.max_time_us
                         << std::setw(12) << result.iterations << "\n";
            }
        }

        /**
         * @brief Compares two benchmark results by name.
         */
        void compare(const std::string& name1, const std::string& name2) const {
            // Raw pointers are appropriate here: they're used as nullable references
            // to search for results in the vector. The vector owns the data, and these
            // pointers are only valid within this function scope. Using std::optional<std::reference_wrapper>
            // would be more complex without adding value for this simple search pattern.
            const Result* r1 = nullptr;
            const Result* r2 = nullptr;

            for (const auto& r : _results) {
                if (r.name == name1) r1 = &r;
                if (r.name == name2) r2 = &r;
            }

            if (!r1 || !r2) {
                std::cout << "Cannot compare: one or both tests not found\n";
                return;
            }

            std::cout << "\n=== COMPARISON ===\n";
            std::cout << name1 << " vs " << name2 << ":\n";

            double speedup = r1->avg_time_us / r2->avg_time_us;
            if (speedup > 1.0) {
                std::cout << name2 << " is " << speedup << "x faster\n";
            } else {
                std::cout << name1 << " is " << (1.0 / speedup) << "x faster\n";
            }
        }

        /**
         * @brief Clears all stored results.
         */
        void clear() {
            _results.clear();
        }

        /**
         * @brief Returns all benchmark results.
         */
        const std::vector<Result>& get_results() const {
            return _results;
        }

    private:
        std::vector<Result> _results;
    };

} // namespace ECS

#endif // ECS_UTILS_BENCHMARK_HPP

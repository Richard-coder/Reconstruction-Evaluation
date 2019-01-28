#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <chrono>

#include "eval/Mesh.h"
#include "eval/PointCloud.h"
#include "eval/Helpers.h"
#include "eval/EvaluationStats.h"

int main(int argc, char** argv) {

    // Folders and filenames
    std::string root_folder = "../dataset/sod/";
    std::string meshes_folder = "meshes/";
    std::string result_filename = "evaluation.txt";

    std::string ref_filename = "ref.ply";
    int rec_min = 3;
    int rec_max = 72;
    std::vector<std::string> rec_filenames;
    for (int i = rec_min; i <= rec_max; i++) {
        std::stringstream ss;
        ss << std::setw(3) << std::setfill('0') << std::to_string(i);
        rec_filenames.emplace_back(ss.str() + ".ply");
    }

    // Evaluation parameters
    int ref_samples = 30000;
    int rec_sample_mult = 2;

    double accuracy_percentage = 0.95;
    double completeness_tolerance_mult = 3.0;

    // Perform evaluation
    EvaluationStats eval_stats(ref_filename, ref_samples);
    Mesh ref_mesh = ReadPly(root_folder + meshes_folder + ref_filename);
    PointCloud ref_pc = ref_mesh.Sample(ref_samples);

    std::cout << "Performing evaluation: " << std::endl;
    auto time_begin = std::chrono::steady_clock::now();

    for (const auto& rec_filename : rec_filenames) {
        std::cout << "\tEvaluating mesh " << rec_filename << std::endl;

        Mesh rec_mesh = ReadPly(root_folder + meshes_folder + rec_filename);
        int rec_samples = rec_mesh.NumVertices() * rec_sample_mult;
        PointCloud rec_pc = rec_mesh.Sample(rec_samples);

        // Rec to ref
        std::vector<float> rec_to_ref = ref_pc.ComputeDistance(rec_pc);
        double rec_to_ref_dist = MeanDistance(rec_to_ref);
        double accuracy = AccuracyMeasure(rec_to_ref, accuracy_percentage);

        // Ref to rec
        double completeness_tolerance = accuracy * completeness_tolerance_mult;
        std::vector<float> ref_to_rec = rec_pc.ComputeDistance(ref_pc);
        double ref_to_rec_dist = MeanDistance(ref_to_rec);
        double completeness = CompletenessMeausre(ref_to_rec, completeness_tolerance);

        // Add to stats
        eval_stats.AddMeshComparison(rec_filename, rec_samples,
                                     rec_to_ref_dist, accuracy,
                                     ref_to_rec_dist, completeness);
    }

    auto time_end = std::chrono::steady_clock::now();
    std::chrono::duration<double> time_elapsed = time_end - time_begin;
    std::cout << "DONE in " << time_elapsed.count() << " s" << std::endl;

    // Write results
    eval_stats.WriteStatsToFile(root_folder + result_filename);
    std::cout << "Stats written to: \n\t" << root_folder + result_filename;

    return EXIT_SUCCESS;
}
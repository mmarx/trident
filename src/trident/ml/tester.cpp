#include <trident/ml/tester.h>
#include <trident/ml/transetester.h>
#include <trident/utils/batch.h>

void Predictor::launchPrediction(KB &kb, string algo, PredictParams &p) {
    //TODO load model
    std::shared_ptr<Embeddings<double>> E;
    std::shared_ptr<Embeddings<double>> R;

    //Load test files
    std::vector<uint64_t> testset;
    string pathtest;
    if (p.nametestset == "valid") {
        pathtest = BatchCreator::getValidPath(kb.getPath());
    } else {
        pathtest = BatchCreator::getTestPath(kb.getPath());
    }
    BatchCreator::loadTriples(pathtest, testset);

    if (algo == "transe") {
        TranseTester<double> tester(E,R);
        auto result = tester.test(p.nametestset, testset, p.nthreads, 0);
    } else {
        BOOST_LOG_TRIVIAL(error) << "Not yet supported";
    }

}
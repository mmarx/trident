#include <trident/utils/batch.h>
#include <trident/kb/kb.h>
#include <trident/kb/querier.h>

#include <boost/filesystem.hpp>

#include <fstream>
#include <algorithm>
#include <random>

namespace fs = boost::filesystem;

BatchCreator::BatchCreator(string kbdir, uint64_t batchsize,
        uint16_t nthreads) : kbdir(kbdir), batchsize(batchsize), nthreads(nthreads) {
    rawtriples = NULL;
    ntriples = 0;
    currentidx = 0;
}

void BatchCreator::createInputForBatch() {
    //Create a file called '_batch' in the maindir with a fixed-length record size
    BOOST_LOG_TRIVIAL(info) << "Store the input for the batch process in " << kbdir + "/_batch ...";
    KBConfig config;
    KB kb(kbdir.c_str(), true, false, false, config);
    Querier *q = kb.query();
    auto itr = q->get(IDX_SPO, -1, -1, -1);
    long s,p,o;
    ofstream ofs;
    ofs.open(this->kbdir + "/_batch", ios::out | ios::app | ios::binary);
    while (itr->hasNext()) {
        itr->next();
        s = itr->getKey();
        p = itr->getValue1();
        o = itr->getValue2();
        const char *cs = (const char*)&s;
        const char *cp = (const char*)&p;
        const char *co = (const char*)&o;
        ofs.write(cs, 5); //Max numbers have 5 bytes
        ofs.write(cp, 5);
        ofs.write(co, 5);
    }
    q->releaseItr(itr);
    ofs.close();
    delete q;
    BOOST_LOG_TRIVIAL(info) << "Done";
}

void BatchCreator::start() {
    //First check if the file exists
    string fin = this->kbdir + "/_batch";
    if (!fs::exists(fin)) {
        BOOST_LOG_TRIVIAL(info) << "Could not find the input file for the batch. I will create it and store it in a file called '_batch'";
        createInputForBatch();
    }

    //Load the file into a memory-mapped file
    this->mapping = bip::file_mapping(fin.c_str(), bip::read_only);
    this->mapped_rgn = bip::mapped_region(this->mapping, bip::read_only);
    this->rawtriples = static_cast<char*>(this->mapped_rgn.get_address());
    this->ntriples = (uint64_t)this->mapped_rgn.get_size() / 15; //triples

    BOOST_LOG_TRIVIAL(info) << "Creating index array ...";
    this->indices.resize(this->ntriples);
    for(long i = 0; i < this->ntriples; ++i) {
        this->indices[i] = i;
    }

    BOOST_LOG_TRIVIAL(info) << "Shuffling array ...";
    auto engine = std::default_random_engine{};
    std::shuffle(this->indices.begin(), this->indices.end(), engine);
    this->currentidx = 0;
    BOOST_LOG_TRIVIAL(info) << "Done";
}

bool BatchCreator::getBatch(std::vector<uint64_t> &output) {
    //The output vector is already supposed to contain batchsize elements. Otherwise, resize it
    output.resize(this->batchsize * 3);

    long i = 0;
    while (i < this->batchsize && this->currentidx < this->ntriples) {
        long s = *(long*)(this->rawtriples + currentidx * 15);
        s = s & 0xFFFFFFFFFFl;
        long p = *(long*)(this->rawtriples + currentidx * 15 + 5);
        p = p & 0xFFFFFFFFFFl;
        long o = *(long*)(this->rawtriples + currentidx * 15 + 10);
        o = o & 0xFFFFFFFFFFl;
        output[i*3] = s;
        output[i*3+1] = p;
        output[i*3+2] = o;
        i+=1;
        this->currentidx++;
    }
    if (i < this->batchsize) {
        output.resize(i);
    }
    return i > 0;
}
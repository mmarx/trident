#include <trident/kb/querier.h>
#include <trident/kb/kb.h>

#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <thread>

using namespace std;


void testKB(Querier *q, std::vector<uint64_t> *v, std::vector<uint64_t> *v1) {
    for(int j = 0; j < 10; j++) {
        int begin = 0;
        int64_t prevs = -1;
        for(int i = 0; i < v->size(); i+=3) {
            int64_t s = v->at(i);
            int64_t p = v->at(i+1);
            int64_t o = v->at(i+2);
            if (prevs != s) {
                if (i != begin) {
                    //Test the KB
                    auto itr = q->get(IDX_SPO, prevs, -1, -1);
                    int tmp = begin;
                    while (itr->hasNext()) {
                        itr->next();
                        int64_t currentp = itr->getValue1();
                        int64_t currento = itr->getValue2();
                        if (currentp != v->at(tmp + 1) || currento != v->at(tmp + 2)) {
                            cout << "Error! s=" << prevs << " p=" << v->at(tmp+1) << " currentp=" << currentp << " o=" << currento << " currento=" << v->at(tmp+2) << endl;
                            exit(1);
                        } else {
                            //cout << prevs << " " << currentp << " " << currento << endl;
                        }
                        tmp += 3;
                    }
                    if (tmp != i) {
                        cout << "Error2!" << endl;
                    }
                    q->releaseItr(itr);
                }
                prevs = s;
                begin = i;
            }
        }
        begin = 0;
        int64_t prevo = -1;
        for(int i = 0; i < v1->size(); i+=3) {
            int64_t s = v1->at(i);
            int64_t p = v1->at(i+1);
            int64_t o = v1->at(i+2);
            if (prevo != o) {
                if (i != begin) {
                    //Test the KB
                    auto itr = q->get(IDX_OPS, -1, -1, prevo);
                    int tmp = begin;
                    while (itr->hasNext()) {
                        itr->next();
                        int64_t currentp = itr->getValue1();
                        int64_t currents = itr->getValue2();
                        if (currentp != v1->at(tmp + 1) || currents != v1->at(tmp)) {
                            cout << "Error! itr->getKey()" << itr->getKey() << " itr->getValue1()=" << currentp << " itr->getValue2()=" << currents << " s=" << v1->at(tmp) << " p=" << v1->at(tmp+1) << " o=" << v1->at(tmp+2) << endl;
                            exit(1);
                        } else {
                            //cout << prevo << " " << currentp << " " << currents << endl;
                        }
                        tmp += 3;
                    }
                    if (tmp != i) {
                        cout << "Error2!" << endl;
                    }
                    q->releaseItr(itr);
                }
                prevo = o;                
                begin = i;
            }
        }
    }
    delete q;
}

int main(int argc, const char** argv) {
    KBConfig config;
    KB kb(argv[1], true, false, false, config);
    //Load the triples
    std::vector<uint64_t> v;
    Querier *q = kb.query();
    auto itr = q->get(IDX_SPO, -1, -1, -1);
    while (itr->hasNext()) {
        itr->next();
        int64_t s = itr->getKey();
        int64_t p = itr->getValue1();
        int64_t o = itr->getValue2();
        v.push_back(s);
        v.push_back(p);
        v.push_back(o);
    }
    q->releaseItr(itr);
    std::vector<uint64_t> v1;
    itr = q->get(IDX_OPS, -1, -1, -1);
    while (itr->hasNext()) {
        itr->next();
        int64_t s = itr->getKey();
        int64_t p = itr->getValue1();
        int64_t o = itr->getValue2();
        v1.push_back(o);
        v1.push_back(p);
        v1.push_back(s);
    }
    q->releaseItr(itr);
    delete q;
    cout << "got " << v.size() << " triples" << endl;

    std::vector<int> nthreads;
    nthreads.push_back(1);
    nthreads.push_back(2);
    nthreads.push_back(3);
    nthreads.push_back(4);
    nthreads.push_back(5);
    nthreads.push_back(6);
    nthreads.push_back(7);
    nthreads.push_back(8);
    for(int i = 0; i < nthreads.size(); ++i) {
        std::vector<std::thread> threads;
        int nt = nthreads[i];
        cout << "Testing with " << nt << " threads" << endl;
        for (int j = 0; j < nt; ++j) {
            Querier *q = kb.query();
            threads.push_back(std::thread(&testKB, q, &v, &v1));
        }
        for(int j = 0; j < nt; ++j) {
            threads[j].join();
        }
    }
    return 0;
}

#include <iostream>
#include "include/function.h"
#include "learnedindex/learned_index.h"
#include "learnedindex/learned_index_impl.h"

struct alignas(CACHELINE_SIZE) ThreadParam;


typedef ThreadParam thread_param_t;
typedef LearnedIndex<key_type, val_type> learnedindex_type;

volatile bool running = false;
std::atomic<size_t> ready_threads(0);

struct alignas(CACHELINE_SIZE) ThreadParam {
    learnedindex_type *li;
    uint64_t throughput;
    uint32_t thread_id;
};

void run_benchmark(learnedindex_type *li, size_t sec);
void *run_fg(void *param);
void prepare(learnedindex_type *&li);

int main(int argc, char **argv) {
    parse_args(argc, argv);
    load_data();
    learnedindex_type *li;
    prepare(li);
    run_benchmark(li, Config.runtime);
    if(li!=nullptr) delete li;
    return 0;
}

void prepare(learnedindex_type *&li){
    COUT_THIS("[Training aidel]");
    double time_s = 0.0;
    TIMER_DECLARE(0);
    TIMER_BEGIN(0);
    size_t maxErr = 4;
    li = new learnedindex_type();
    li->train(exist_keys, exist_keys, 250000, 64);
    TIMER_END_S(0,time_s);
    printf("%8.1lf s : %.40s\n", time_s, "training");
}

void run_benchmark(learnedindex_type *li, size_t sec) {
    pthread_t threads[Config.thread_num];
    thread_param_t thread_params[Config.thread_num];
    // check if parameters are cacheline aligned
    for (size_t i = 0; i < Config.thread_num; i++) {
        if ((uint64_t)(&(thread_params[i])) % CACHELINE_SIZE != 0) {
            COUT_N_EXIT("wrong parameter address: " << &(thread_params[i]));
        }
    }

    running = false;
    for(size_t worker_i = 0; worker_i < Config.thread_num; worker_i++){
        thread_params[worker_i].li = li;
        thread_params[worker_i].thread_id = worker_i;
        thread_params[worker_i].throughput = 0;
        int ret = pthread_create(&threads[worker_i], nullptr, run_fg,
                                (void *)&thread_params[worker_i]);
        if (ret) {
            COUT_N_EXIT("Error:" << ret);
        }
    }

    COUT_THIS("[micro] prepare data ...");
    while (ready_threads < Config.thread_num) sleep(0.5);

    running = true;
    std::vector<size_t> tput_history(Config.thread_num, 0);
    size_t current_sec = 0;
    while (current_sec < sec) {
        sleep(1);
        uint64_t tput = 0;
        for (size_t i = 0; i < Config.thread_num; i++) {
            tput += thread_params[i].throughput - tput_history[i];
            tput_history[i] = thread_params[i].throughput;
        }
        COUT_THIS("[micro] >>> sec " << current_sec << " throughput: " << tput);
        ++current_sec;
    }

    running = false;
    void *status;
    for (size_t i = 0; i < Config.thread_num; i++) {
        int rc = pthread_join(threads[i], &status);
        if (rc) {
            COUT_N_EXIT("Error:unable to join," << rc);
        }
    }

    size_t throughput = 0;
    for (auto &p : thread_params) {
        throughput += p.throughput;
    }
    COUT_THIS("[micro] Throughput(op/s): " << throughput / sec);
}

void *run_fg(void *param) {
    thread_param_t &thread_param = *(thread_param_t *)param;
    uint32_t thread_id = thread_param.thread_id;
    learnedindex_type *li = thread_param.li;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> ratio_dis(0, 1);

    size_t non_exist_key_n_per_thread = non_exist_keys.size() / Config.thread_num;
    size_t non_exist_key_start = thread_id * non_exist_key_n_per_thread;
    size_t non_exist_key_end = (thread_id + 1) * non_exist_key_n_per_thread;
    std::vector<key_type> op_keys(non_exist_keys.begin() + non_exist_key_start,
                                   non_exist_keys.begin() + non_exist_key_end);

    COUT_THIS("[micro] Worker" << thread_id << " Ready.");
    size_t query_i = 0, insert_i = 0, delete_i = 0, update_i = 0;
    // exsiting keys fall within range [delete_i, insert_i)
    ready_threads++;
    volatile bool res = false;
    val_type dummy_value = 1234;

    while (!running)
        ;
	while (running) {
        double d = ratio_dis(gen);
        if (d <= Config.read_ratio) {                   // search
            key_type dummy_key = exist_keys[query_i % exist_keys.size()];
            res = li->find(dummy_key, dummy_value);
            query_i++;
            if (unlikely(query_i == exist_keys.size())) {
                query_i = 0;
            }
        } else if (d <= Config.read_ratio+Config.insert_ratio){  // insert
            key_type dummy_key = non_exist_keys[insert_i % non_exist_keys.size()];
            li->insert(dummy_key, dummy_key);
            insert_i++;
            if (unlikely(insert_i == non_exist_keys.size())) {
                insert_i = 0;
            }
        } else if (d <= Config.read_ratio+Config.insert_ratio+Config.update_ratio) {    // update
            key_type dummy_key = non_exist_keys[update_i % non_exist_keys.size()];
            res = li->update(dummy_key, dummy_key);
            update_i++;
            if (unlikely(update_i == non_exist_keys.size())) {
                update_i = 0;
            }
        }  else {                // remove
            key_type dummy_key = exist_keys[delete_i % exist_keys.size()];
            res = li->remove(dummy_key);
            delete_i++;
            if (unlikely(delete_i == exist_keys.size())) {
                delete_i = 0;
            }
        }
        thread_param.throughput++;
    }
    pthread_exit(nullptr);
}


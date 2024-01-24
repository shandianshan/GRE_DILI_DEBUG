#include"src/src/DyTIS_impl.h"
#include"../indexInterface.h"

// @todo
// maybe need to convert class DyTIS into a template class
template<class KEY_TYPE, class PAYLOAD_TYPE>
class DYTISInterface : public indexInterface<KEY_TYPE, PAYLOAD_TYPE> {
public:
    void init(Param *param = nullptr) {}

    void bulk_load(std::pair <KEY_TYPE, PAYLOAD_TYPE> *key_value, size_t num, Param *param = nullptr);

    bool get(KEY_TYPE key, PAYLOAD_TYPE &val, Param *param = nullptr);

    bool put(KEY_TYPE key, PAYLOAD_TYPE value, Param *param = nullptr);

    bool update(KEY_TYPE key, PAYLOAD_TYPE value, Param *param = nullptr);

    bool remove(KEY_TYPE key, Param *param = nullptr);

    size_t scan(KEY_TYPE key_low_bound, size_t key_num, std::pair <KEY_TYPE, PAYLOAD_TYPE> *result,
                Param *param = nullptr);

    // @todo
    long long memory_consumption() { 
        //return lipp.total_size(); 
        return 0;
    }

private:
    dytis::DyTIS dytis_;
};

template<class KEY_TYPE, class PAYLOAD_TYPE>
void DYTISInterface<KEY_TYPE, PAYLOAD_TYPE>::bulk_load(std::pair <KEY_TYPE, PAYLOAD_TYPE> *key_value, size_t num,
                                                      Param *param) {
    for(int i=0 ;i< num ;i++) {
        dytis_.Insert((key_value+i)->first,(key_value+i)->second);
    }                                     
    // @todo
   // dytis_.bulk_load(key_value, static_cast<int>(num));
}

template<class KEY_TYPE, class PAYLOAD_TYPE>
bool DYTISInterface<KEY_TYPE, PAYLOAD_TYPE>::get(KEY_TYPE key, PAYLOAD_TYPE &val, Param *param) {
    bool exist=false;
    //val = *dytis.Find(key);
    // actually below is also ok as LIPP not consider class object as key/value
    val = dytis_.Get(key);
    if(val == dytis::NONE) {
        exist = false;
    }else {
        exist = true;
    }
    return exist;
}

template<class KEY_TYPE, class PAYLOAD_TYPE>
bool DYTISInterface<KEY_TYPE, PAYLOAD_TYPE>::put(KEY_TYPE key, PAYLOAD_TYPE value, Param *param) {
    // @todo why dytis.Insert use reference as param?
    dytis_.Insert(key, value);

    // always true, just like lipp
    return true;

}

template<class KEY_TYPE, class PAYLOAD_TYPE>
bool DYTISInterface<KEY_TYPE, PAYLOAD_TYPE>::update(KEY_TYPE key, PAYLOAD_TYPE value, Param *param) {
    return dytis_.Update(key, value);
}

template<class KEY_TYPE, class PAYLOAD_TYPE>
bool DYTISInterface<KEY_TYPE, PAYLOAD_TYPE>::remove(KEY_TYPE key, Param *param) {
    // @todo do not understand the return value of delete 
    return dytis_.Delete(key);
}

template<class KEY_TYPE, class PAYLOAD_TYPE>
size_t DYTISInterface<KEY_TYPE, PAYLOAD_TYPE>::scan(KEY_TYPE key_low_bound, size_t key_num,
                                                   std::pair <KEY_TYPE, PAYLOAD_TYPE> *result,
                                                   Param *param) {
    if(!result) {
        result = new std::pair <KEY_TYPE, PAYLOAD_TYPE>[key_num];
    }
    // @todo may need to modify Scan
    // pass in pair * result and return count
    PAYLOAD_TYPE *values = NULL; 
    dytis_.Scan(key_low_bound, key_num);
    // ignore result, if need, modify Scan in the future
    return key_num;
}


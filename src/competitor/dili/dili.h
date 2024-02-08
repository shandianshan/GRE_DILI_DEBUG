#include<vector>
#include"./src/src/dili/DILI.h"
#include"../indexInterface.h"
#include"./src/src/utils/data_utils.h"
#include <filesystem>

template<class KEY_TYPE, class PAYLOAD_TYPE>
class diliInterface : public indexInterface<KEY_TYPE, PAYLOAD_TYPE> {
public:
  void init(Param *param = nullptr) {}

  void bulk_load(std::pair <KEY_TYPE, PAYLOAD_TYPE> *key_value, size_t num, Param *param = nullptr);

  bool get(KEY_TYPE key, PAYLOAD_TYPE &val, Param *param = nullptr); 

  bool put(KEY_TYPE key, PAYLOAD_TYPE value, Param *param = nullptr);

  bool update(KEY_TYPE key, PAYLOAD_TYPE value, Param *param = nullptr);

  bool remove(KEY_TYPE key, Param *param = nullptr);

  size_t scan(KEY_TYPE key_low_bound, size_t key_num, std::pair<KEY_TYPE, PAYLOAD_TYPE> *result,
              Param *param = nullptr);

 long long memory_consumption() { return 0; }
/* private:
  dili::Dili<keyType, PAYLOAD_TYPE, diliInterface::AlexCompare, std::allocator < std::pair < KEY_TYPE, PAYLOAD_TYPE>>, false>
  index;   */
  DILI dili;
  
};

template<class KEY_TYPE, class PAYLOAD_TYPE>
void diliInterface<KEY_TYPE, PAYLOAD_TYPE>::bulk_load(std::pair <KEY_TYPE, PAYLOAD_TYPE> *key_value, size_t num,
                                                      Param *param) {
  std::vector< pair<keyType, recordPtr> >  bulk_data;
  std::pair<long ,long> temp;
  
  string mirror_dir = "src/competitor/dili/src/build/data/buTree";
  int status = file_utils::path_status(mirror_dir);
  assert(status != 2);
  if (status == 0) {
      file_utils::detect_and_create_dir(mirror_dir);
  }else { // == 1, is a directory
    // delete data/buTree/ and create it again
    std::filesystem::remove_all(mirror_dir);
    file_utils::detect_and_create_dir(mirror_dir);
  }
  dili.set_mirror_dir(mirror_dir);
  
  long last_tem = -1;
  bool first = true;
  for (unsigned long i = 0; i < num ; i++){
    temp.first=(long)(key_value[i].first);
    temp.second=(long)(key_value[i].second);
    if(!first){
      if(last_tem >= temp.first){
        std::cout << last_tem << " " << temp.first << std::endl;
      }
      assert(last_tem < temp.first);
    }else{
      first = false;
    }
    last_tem = temp.first;
    bulk_data.push_back(temp);
  }                                                     
  dili.bulk_load(bulk_data);
}

template<class KEY_TYPE, class PAYLOAD_TYPE>
bool diliInterface<KEY_TYPE, PAYLOAD_TYPE>::get(KEY_TYPE key, PAYLOAD_TYPE &val, Param *param) {
   PAYLOAD_TYPE res = (unsigned long)(dili.search((long)key));
   if ( res != -1) { // dili.search return -1 when error
     val = res;
     return true;
   }
   return false;
 } 

template<class KEY_TYPE, class PAYLOAD_TYPE>
bool diliInterface< KEY_TYPE, PAYLOAD_TYPE>::put(KEY_TYPE key, PAYLOAD_TYPE value, Param *param) {
  return dili.insert((long)(key), (long)(value));
}

template<class KEY_TYPE, class PAYLOAD_TYPE>
bool diliInterface< KEY_TYPE, PAYLOAD_TYPE>::update(KEY_TYPE key, PAYLOAD_TYPE value, Param *param) {
     //return dili.update(key, value);
     return false;
 }

template<class KEY_TYPE, class PAYLOAD_TYPE>
bool diliInterface<KEY_TYPE, PAYLOAD_TYPE>::remove(KEY_TYPE key, Param *param) {
  auto num_erase = dili.delete_key((long)(key));
  return num_erase > 0;
}

template<class KEY_TYPE, class PAYLOAD_TYPE>
size_t diliInterface<KEY_TYPE, PAYLOAD_TYPE>::scan(KEY_TYPE key_low_bound, size_t key_num,std::pair<KEY_TYPE, PAYLOAD_TYPE> *result,
                                                   Param *param) {
  /* auto iter = index.lower_bound(key_low_bound);
  int scan_size = 0;
  for (scan_size = 0; scan_size < key_num && !iter.is_end(); scan_size++) {
    result[scan_size] = {(*iter).first, (*iter).second};
    iter++;
  } */

  //return dili.range_query(key_low_bound, key_low_bound + key_num, *result); //
  return 0;
}  

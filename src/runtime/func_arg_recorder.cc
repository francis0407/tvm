//
// Created by hmc on 2021/2/26.
//
#include <tvm/runtime/func_arg_recorder.h>
#include <dmlc/json.h>

namespace tvm {
namespace runtime {


FunctionArgRecorder global_recorder;

PackedFunc FunctionArgRecorder::GetFunction(const std::string& name, const ObjectPtr<Object>& sptr_to_self) {
  return PackedFunc();
}

void FunctionArgRecorder::SaveToFile(const std::string& file_name, const std::string& format) {

}

void FunctionArgRecorder::NewHostRecord(std::string name, std::vector<void*> args) {
  HostRecord r;
  r.host_func_name = name;
  for (size_t i = 0; i < args.size(); i++)
    r.host_args[args[i]] = i;
  records.push_back(r);
}

void FunctionArgRecorder::AllocArg(void* arg, size_t size) {
  if (records.empty()) return;
  HostRecord &r = records.back();
  r.alloc_args[arg] = size;
}

void FunctionArgRecorder::NewDeviceRecord(std::string name, std::vector<void*> args, size_t launch_params[6]) {
  if (records.empty()) return;
  HostRecord &r = records.back();
  DeviceRecord dr;
  dr.device_func_name = name;
  dr.device_args = args;
  for (int i = 0; i < 6; i++)
    dr.launch_params[i] = launch_params[i];
  r.device_records.push_back(dr);
}

void FunctionArgRecorder::Print() {
//  for (HostRecord &hr : records) {
//    printf("==========================");
//    printf("Host: %s\nHostArgs: ", hr.host_func_name.c_str());
//    for (size_t i = 0; i < hr.host_args.size(); i++) {
//      if (i != 0) printf(",");
//      for (auto arg: hr.host_args) {
//        if (arg.second == i) {
//          printf("%p", arg.first);
//        }
//      }
//    }
//    printf("\nAllocArgs: ");
//    size_t i = 0;
//    for (auto arg: hr.alloc_args) {
//      if (i != 0) printf(",");
//      printf("[%lu]%p", arg.second, arg.first);
//      i++;
//    }
//    for (auto &dev : hr.device_records) {
//      printf("\nDevice: %s\nDeviceArgs: ", dev.device_func_name.c_str());
//      for (i = 0; i < dev.device_args.size(); i++) {
//        if (i != 0) printf(",");
//        printf("%p", dev.device_args[i]);
//      }
//    }
//    printf("\n");
//  }
    printf("%s\n", ToJson().c_str());
}

//
//  {
//      "temp_args": [1024, 2048], // size
//      "funcs": [
//          {
//            "name": "fused_add_10",
//            "kernels": [
//              {
//                "name": "fused_add_10_kernel0",
//                "launch_params": [147, 1, 1, 1024, 1, 1],
//                "args": [1, -1, 0] // 0, 1 means the first arg of fused_add_10; -1 means the first arg of temp_args
//              }
//            ]
//          },
//      ]
//  }
std::string FunctionArgRecorder::ToJson() {

  std::vector<size_t> alloc_args = getTempArgs();
  prepare_device_arg_idx();

  std::ostringstream os;
  dmlc::JSONWriter writer(&os);
  writer.BeginObject(true);
  writer.WriteObjectKeyValue("temp_args", alloc_args);
  writer.WriteObjectKeyValue("funcs", records);
  writer.EndObject();

//  for (size_t i = 0; i < this->records.size(); i++) {
//    auto &r = records[i];
//
//  }
  return os.str();
}




} // namespace runtime

} // namespace tvm
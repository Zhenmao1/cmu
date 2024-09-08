//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lru_k_replacer.cpp
//
// Identification: src/buffer/lru_k_replacer.cpp
//
// Copyright (c) 2015-2022, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/lru_k_replacer.h"
#include "common/exception.h"

namespace bustub {

LRUKReplacer::LRUKReplacer(size_t num_frames, size_t k){
    this->replacer_size_=num_frames;
    this->k_=k;
}


/* 
    保证线程安全→上锁
    获取“可驱逐节点”最大的k- distance（需要判断如果小于k的设为+inf并单独先存起来）
    确定待驱逐的页面
    剩下的一些处理：
    清除历史
    将这个页面设置成不可取驱逐
    return
*/
auto LRUKReplacer::Evict(frame_id_t *frame_id) -> bool { 
    std::lock_guard<std::mutex> lock(latch_);
    auto is_inf=false;
    bustub::LRUKNode evit_node;
    if(curr_size_<=0) {
        return false;
    }
    auto success=false;
    size_t max_bkd=0;
    size_t max_lessk_bkd=0;
    for( auto &it:node_store_){
        auto &node=it.second;
        if(!node.is_evictable_){
            continue;
        }
        success=true;
        if(node.history_.empty()){
            *frame_id=it.first;
            evit_node=it.second;
        }
        if(node.history_.size()==k_&&!is_inf){
            auto cur_bkd = current_timestamp_- node.GetBackKTimeStamp();//一定为正
            if(cur_bkd>max_bkd){
                *frame_id=it.first;
                evit_node=it.second;
                max_bkd=cur_bkd;
            }
        }else if(node.history_.size()<k_){
            is_inf=true;
            auto cur_less_bkd = current_timestamp_- node.GetBackKTimeStamp();//一定为正
            if(cur_less_bkd>max_lessk_bkd){
                *frame_id=it.first;
                evit_node=it.second;
                max_lessk_bkd=cur_less_bkd;
            }
        }
        if(success){
            curr_size_--;
            evit_node.history_.clear();
            node_store_.erase(*frame_id);
        }
        return success;


    }
    return false;
}


/****
·检查frame_id是否合法(不超过replacer_size_)
    ·如果合法，在node_store_中find frame_if
        ·如果page已经存在，维护其访问记录history_, 注意k-instace的维护。
            ·如果page可驱逐，且需要更新k-instance，需要先从set中erase，再insert到set中。直接修改LRUKNode不会触发set的调整，所以你只能手动删除再插入
        ·如果page不存在，make_shared, 向node_store_中插入新的访问记录，当然你需要对LRUKNode进行初始化
****/

void LRUKReplacer::RecordAccess(frame_id_t frame_id,AccessType access_type) {
    std::lock_guard<std::mutex> lock(latch_);
    if(replacer_size_>static_cast<size_t>(frame_id)){
        //"页框号超出缓存区可容纳的页面数量！"
        throw std::exception();
    }
    //当前时间戳++
    current_timestamp_++;
    if(node_store_.find(frame_id)==node_store_.end()){
        auto new_node_ptr=std::make_shared<LRUKNode>();
        if(access_type!=AccessType::Scan){
            new_node_ptr->history_.push_back(current_timestamp_);
        }
        node_store_.insert(std::make_pair(frame_id,*new_node_ptr));
    }else{
        auto node=node_store_[frame_id];
        if(access_type!=AccessType::Scan){
            node.history_.push_back(current_timestamp_);
            if(node.history_.size()>k_){
                node.history_.pop_front();
            }
        }
        
    }
}

void LRUKReplacer::SetEvictable(frame_id_t frame_id, bool set_evictable) {

    std::lock_guard<std::mutex> lock(latch_);
    if(replacer_size_<static_cast<size_t>(frame_id)){
        throw std::out_of_range("frame_id exceeds replacer size in set_evict");
    }
    if(node_store_.find(frame_id)==node_store_.end()){
        auto new_node=std::make_unique<LRUKNode>();
        node_store_.emplace(std::make_pair(frame_id,new_node));
    }
    auto& node=node_store_[frame_id];
    
    if(node.is_evictable_!=set_evictable){
        if(set_evictable){
            curr_size_++;
        }else{
            curr_size_--;
        }
        node.is_evictable_=set_evictable;
    }
}

void LRUKReplacer::Remove(frame_id_t frame_id) {
    std::lock_guard<std::mutex> lock(latch_);
    if(replacer_size_<static_cast<size_t>(frame_id)){
        throw std::out_of_range("frame_id exceeds replacer size in remove");
    }
    if(node_store_.find(frame_id)==node_store_.end()){
        return;
    }
    auto& node=node_store_[frame_id];
    if(!node.is_evictable_){
        throw std::exception();
    }
    node_store_.erase(frame_id);
    node.history_.clear();
    curr_size_--;

}

auto LRUKReplacer::Size() -> size_t { return curr_size_; }

}  // namespace bustub

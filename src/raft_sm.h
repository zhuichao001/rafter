#ifndef _RAFT_STATE_MACHINE_H_
#define _RAFT_STATE_MACHINE_H_

#include <string>
#include <stdint.h>
#include "proto/raftmsg.pb.h"

typedef struct {
    int action; //-1:remove, 1:add
    int nodeid;
    int peerid;
    int type;   //0:normal, 1:leaner
} ConfChange;

class RaftStateMachine{
public:
    virtual int Apply(const std::string data) = 0;
    virtual int ApplyMemberAdd(const raft::Peer &peer) = 0;
    virtual int ApplyMemberDel(const raft::Peer &peer) = 0;
    virtual uint64_t GetAppliedIndex() = 0;
    virtual int OnTransferLeader(bool isleader) = 0;
};

#endif

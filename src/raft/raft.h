#ifndef _RAFT_RAFT_H_
#define _RAFT_RAFT_H_

#include <memory>
#include <map>
#include <time.h>
#include "raft_sm.h"
#include "raft_node.h"
#include "raft_log.h"
#include "options.h"
#include "transport.h"
#include "proto/raftmsg.pb.h"

enum RAFT_STATE {
    NONE = 0,
    LEADER,
    FOLLOWER,
    CANDIDATE,
    LEANER,
};

class Raft{
public:
    Raft(const RaftOptions &opt);

    int Propose(const std::string &data);

    int ChangeMember(int action, std::string addr); //1:add, -1:remove

private: //for leader
    int appendEntry(raft::LogEntry *e);

    void sendAppendEntries();

    void sendAppendEntries(RaftNode *node);

    int recvAppendEntriesResponse(const raft::AppendEntriesResponse *r);

    void recvHeartbeatResponse(const raft::HeartbeatResponse *rsp);

    RaftNode *addRaftNode(int nodeid, const address_t *addr, bool is_self, bool is_voting=true);

private: //for follower
    void tick();

    void startElection();

    void becomeCandidate();

    bool shouldGrantVote(const raft::VoteRequest* req);

    int voteFor(const int nodeid);

    int recvAppendEntries(const raft::AppendEntriesRequest *msg, raft::AppendEntriesResponse *rsp);

    int recvVoteRequest(const raft::VoteRequest *req, raft::VoteResponse *rsp);

    void recvHeartbeat(const raft::HeartbeatRequest *req, raft::HeartbeatResponse *rsp);

private: //for candidate
    void becomeLeader();

    void becomeFollower();

    int getVotesNum();

    int sendVoteRequest(RaftNode *node);

    int recvVoteResponse(const raft::VoteResponse *rsp);

private:
    int applyEntry();

    void setState(int st) {
        state_ = st;
    }

    uint64_t getCurrentIndex(){
        return log_.getCurrentIndex();
    }

    uint64_t getLastLogTerm(){
        int idx = log_.getCurrentIndex();
        if (idx>0) {
            const raft::LogEntry *e = log_.getEntry(idx);
            if (e) {
                return e->term();
            }
        }
        return 0;
    }

    bool isLeader(){
        return RAFT_STATE::LEADER == state_;
    }

    bool isFollower(){
        return RAFT_STATE::FOLLOWER == state_;
    }

    bool isCandidate(){
        return RAFT_STATE::CANDIDATE == state_;
    }

    bool isAlreadyVoted(){
        return voted_for_ != -1;
    }

private:
    int id_; //raft group id
    RaftLog log_;

    uint64_t term_;
    int voted_for_;
    int state_;         //FOLLOWER,LEADER,CANDIDATE

    uint64_t commit_idx_;
    uint64_t applied_idx_;
    uint64_t reconf_idx_;

    int time_elapsed_;  //since last time
    int timeout_election_;
    int timeout_request_;

    std::map<const int, RaftNode*> nodes_;
    RaftNode *leader_;
    RaftNode *local_;

    RaftStateMachine *app_;
    std::shared_ptr<Transport> trans_;

    friend class Transport;
    friend class RaftServer;
};

#endif

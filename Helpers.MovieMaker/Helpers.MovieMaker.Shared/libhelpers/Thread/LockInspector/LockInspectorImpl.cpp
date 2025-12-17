#include "pch.h"
#include "LockInspectorImpl.h"
#include "..\..\HSystem.h"
#include "..\..\HTime.h"

#include <assert.h>

LockInspectorImpl *LockInspectorImpl::Instance() {
    static LockInspectorImpl instance;
    return &instance;
}

void LockInspectorImpl::OnLock(void *csAddress) {
    LockInspEvent item(csAddress, LockInspEventType::Lock);

    {
        std::unique_lock<std::mutex> lk(this->lkEventsMtx);
        this->lkEventsProd.push_back(std::move(item));
    }

    this->lkEventsCv.notify_one();
}

void LockInspectorImpl::OnUnlock(void *csAddress) {
    LockInspEvent item(csAddress, LockInspEventType::Unlock);

    {
        std::unique_lock<std::mutex> lk(this->lkEventsMtx);
        this->lkEventsProd.push_back(std::move(item));
    }

    this->lkEventsCv.notify_one();
}

void LockInspectorImpl::OnLockDestructor(void *csAddress) {
    LockInspEvent item(csAddress, LockInspEventType::Destructor);

    {
        std::unique_lock<std::mutex> lk(this->lkEventsMtx);
        this->lkEventsProd.push_back(std::move(item));
    }

    this->lkEventsCv.notify_one();
}

void LockInspectorImpl::SetCurrentThreadForBreak() {
    std::unique_lock<std::mutex> lk(this->breakThreadIdMtx);
    this->breakThreadId = std::this_thread::get_id();
}

void LockInspectorImpl::ResetThreadForBreak() {
    std::unique_lock<std::mutex> lk(this->breakThreadIdMtx);
    this->breakThreadId = std::thread::id();
}

LockInspectorImpl::LockInspectorImpl() {
    this->exit = false;
    const size_t ReserveCount = 200000;

    this->lkEventsProd.reserve(ReserveCount);
    this->lkEventsCons.reserve(ReserveCount);

    this->treeItemsProd.reserve(ReserveCount);
    this->treeItemsCons.reserve(ReserveCount);

    this->treeReportProd.reserve(ReserveCount);
    this->treeReportCons.reserve(ReserveCount);

    this->lkEventsThread = std::thread([this]() { this->ThreadMain(); });
    this->treeThread = std::thread([this]() { this->TreeThreadMain(); });
    this->treeReportThread = std::thread([this]() { this->TreeReportThreadMain(); });
}

LockInspectorImpl::~LockInspectorImpl() {
    this->exit = true;

    this->lkEventsCv.notify_one();
    this->treeItemsCv.notify_one();
    this->treeReportCv.notify_one();

    if (this->lkEventsThread.joinable()) {
        this->lkEventsThread.join();
    }

    if (this->treeThread.joinable()) {
        this->treeThread.join();
    }

    if (this->treeReportThread.joinable()) {
        this->treeReportThread.join();
    }
}

void LockInspectorImpl::ThreadMain() {
    while (true) {
        {
            std::unique_lock<std::mutex> lk(this->lkEventsMtx);

            while (this->lkEventsProd.empty() && !this->exit) {
                this->lkEventsCv.wait(lk);
            }

            std::swap(this->lkEventsProd, this->lkEventsCons);
            this->lkEventsProd.clear();

            if (this->exit) {
                break;
            }
        }

        for (auto &i : this->lkEventsCons) {
            this->ProcessItem(std::move(i));
        }
        this->lkEventsCons.clear();
    }
}

void LockInspectorImpl::ProcessItem(LockInspEvent &&item) {
    {
        std::unique_lock<std::mutex> lk(this->breakThreadIdMtx);
        if (this->breakThreadId == item.GetThreadId()) {
            int stop = 342;
        }
    }

    LockTreeAction treeAction;

    if (item.GetType() == LockInspEventType::Destructor) {
        /*auto checkFn = [&]() {
            std::unique_lock<std::mutex> lk(this->treeMtx);
            auto threadData = &this->curThreadTree[item.GetThreadId()];
            return threadData->AtRoot();
        };
        assert(checkFn());*/

        treeAction.type = LockTreeActionType::Delete;
        treeAction.deleteData.evt = std::move(item);
    }
    else {
        bool itemLock = item.GetType() == LockInspEventType::Lock;
        auto itemThreadId = item.GetThreadId();

        std::unique_lock<std::mutex> lk(this->treeMtx);
        auto threadData = &this->curThreadTree[item.GetThreadId()];
        threadData->Add(std::move(item));

        if (threadData->depth > 1000) {
            int stop = 432;
        }

        bool clear = false;

        if (threadData->tree.failed) {
            int stop = 234;
            clear = true;
        }

        if (!itemLock && threadData->AtRoot() && !threadData->tree.failed) {
            int stop = 342;

            clear = true;
        }

        if (clear) {
            if (!threadData->tree.failed) {
                treeAction.type = LockTreeActionType::Merge;
                treeAction.mergeData.tree = threadData->tree;
            }

            this->curThreadTree.erase(itemThreadId);
        }
    }

    if (treeAction.type != LockTreeActionType::None) {
        {
            std::unique_lock<std::mutex> lk(this->treeItemsMtx);
            this->treeItemsProd.push_back(std::move(treeAction));
        }
        this->treeItemsCv.notify_one();
    }
}

void LockInspectorImpl::TreeThreadMain() {
    while (true) {
        {
            std::unique_lock<std::mutex> lk(this->treeItemsMtx);

            while (this->treeItemsProd.empty() && !this->exit) {
                this->treeItemsCv.wait(lk);
            }

            std::swap(this->treeItemsProd, this->treeItemsCons);
            this->treeItemsProd.clear();

            if (this->exit) {
                break;
            }
        }

        this->ProcessTreeItems();
        this->treeItemsCons.clear();
    }
}

void LockInspectorImpl::ProcessTreeItems() {
    bool updated = false;
    LockTreeActionType prevAction = LockTreeActionType::None;
    auto sendTreeToReport = [&]() {
        if (prevAction != LockTreeActionType::None && prevAction != LockTreeActionType::Delete && updated) {
            updated = false;

            {
                std::unique_lock<std::mutex> lk(this->treeReportMtx);
                this->treeReportProd.push_back(this->lockTree);
                this->treeReportProd.back().CheckParent();
            }
            this->treeReportCv.notify_one();
        }
    };

    for (auto &i : this->treeItemsCons) {
        auto action = i.type;

        switch (action) {
        case LockTreeActionType::Merge: {
            updated = this->lockTree.MergeToNodes(i.mergeData.tree);
            this->lockTree.CheckParent();
            break;
        }
        case LockTreeActionType::Delete: {
            sendTreeToReport();
            this->lockTree.RemoveLockRecursive(i.deleteData.evt.GetValue());
            break;
        }
        default:
            break;
        }

        prevAction = action;
    }

    sendTreeToReport();
}

void LockInspectorImpl::TreeReportThreadMain() {
    this->lockTreeLastUpdate = H::Time::GetCurrentLibTime();

    while (true) {
        {
            std::unique_lock<std::mutex> lk(this->treeReportMtx);

            while (this->treeReportProd.empty() && !this->exit && !this->ItIsTimeToReport()) {
                this->treeReportCv.wait_for(lk, std::chrono::milliseconds(250));
            }

            if (this->treeReportCons.empty()) {
                std::swap(this->treeReportProd, this->treeReportCons);
                this->treeReportProd.clear();
            }

            if (this->exit) {
                break;
            }
        }

        if (this->ItIsTimeToReport()) {
            for (auto &i : this->treeReportCons) {
                this->Report(i);
            }

            this->treeReportCons.clear();
            this->lockTreeLastUpdate = H::Time::GetCurrentLibTime();
        }
    }
}

bool LockInspectorImpl::ItIsTimeToReport() {
    bool res = H::Time::GetCurrentLibTime() - this->lockTreeLastUpdate > H::Time::HNSResolution * 2;
    return res;
}

void LockInspectorImpl::Report(const LockInspTreeNode &lkTree) {
    auto intersect = [](const std::vector<void *> &a, const std::vector<void *> &b, std::vector<void *> &Aintersection) {
        for (auto &i : a) {
            for (auto &j : b) {
                if (i == j) {
                    Aintersection.push_back(i);
                    break;
                }
            }
        }
    };

    struct BranchStats {
        bool done;
        size_t branchIdx;
        std::vector<size_t> collisionList;
    };

    std::vector<void *> branch0, branch1;
    std::vector<void *> branch0Intersection, branch1Intersection;
    std::vector<BranchStats> stats;
    auto it0 = lkTree.GetFirstBranchIterator();
    size_t it0Idx = 0;

    size_t branchCount = 0;

    while (!it0.IsEnd()) {
        branchCount++;
        it0.Next();
    }

    stats.reserve(branchCount);

    for (size_t i = 0; i < branchCount; i++) {
        BranchStats stat;

        stat.branchIdx = i;
        stat.done = false;
        stats.push_back(stat);
    }

    it0 = lkTree.GetFirstBranchIterator();

    while (!it0.IsEnd()) {
        size_t it1Idx = 0;
        auto it1 = lkTree.GetFirstBranchIterator();

        branch0.clear();
        it0.GetBranch(branch0);

        while (!it1.IsEnd()) {
            if (it0 == it1 || stats[it1Idx].done) {
                it1Idx++;
                it1.Next();
                continue;
            }

            branch1.clear();
            branch0Intersection.clear();
            branch1Intersection.clear();
            it1.GetBranch(branch1);

            // analyze branches
            intersect(branch0, branch1, branch0Intersection);
            intersect(branch1, branch0, branch1Intersection);

            if (branch0Intersection.size() != branch1Intersection.size()) {
                //// can break in case of using same lock twice or intersect is wrong
                //auto it0 = std::unique(branch0Intersection.begin(), branch0Intersection.end());
                //auto it1 = std::unique(branch1Intersection.begin(), branch1Intersection.end());
                //branch0Intersection.erase(it0, branch0Intersection.end());
                //branch1Intersection.erase(it1, branch1Intersection.end());

                auto st0 = it0.GetBranchWithStack();
                auto st1 = it1.GetBranchWithStack();

                int stop = 324;
            }

            // can break in case of using same lock twice or intersect is wrong
            assert(branch0Intersection.size() == branch1Intersection.size());

            bool same = true;

            for (size_t i = 0; i < branch0Intersection.size() && same; i++) {
                same = branch0Intersection[i] == branch1Intersection[i];
            }

            if (!same) {
                if (it0Idx == 2 && it1Idx == 16) {
                    int stop = 342;
                }
                stats[it0Idx].collisionList.push_back(it1Idx);
                stats[it1Idx].collisionList.push_back(it0Idx);
            }
            // analyze branches

            it1Idx++;
            it1.Next();
        }

        stats[it0Idx].done = true;
        it0Idx++;
        it0.Next();
    }

    std::sort(stats.begin(), stats.end(), [](const BranchStats &a, const BranchStats &b) {
        return a.collisionList.size() > b.collisionList.size();
    });

    for (auto &stat : stats) {
        if (stat.collisionList.empty()) {
            // stats already sorted in decreasing order of collisionList.size()
            // so we can break
            break;
        }

        auto itThis = lkTree.GetFirstBranchIterator();
        for (size_t i = 0; i < stat.branchIdx; i++) {
            itThis.Next();
        }

        auto branchThis = itThis.GetBranchWithStack();

        branch0.clear();
        itThis.GetBranch(branch0);

        for (auto &otherIdx : stat.collisionList) {
            auto itOther = lkTree.GetFirstBranchIterator();
            for (size_t i = 0; i < otherIdx; i++) {
                itOther.Next();
            }

            branch1.clear();
            branch0Intersection.clear();
            branch1Intersection.clear();
            itOther.GetBranch(branch1);

            // analyze branches
            intersect(branch0, branch1, branch0Intersection);
            intersect(branch1, branch0, branch1Intersection);

            auto branchOther = itOther.GetBranchWithStack();

            int stop = 324;
        }
    }
}
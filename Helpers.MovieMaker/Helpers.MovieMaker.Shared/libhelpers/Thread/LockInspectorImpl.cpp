#include "pch.h"
#include "LockInspectorImpl.h"
#include "..\HSystem.h"

// #define LockInspectorImpl_USE_THREAD

// LockInspTree=====================================================================
LockInspTree::LockInspTree()
    : csAddress(nullptr), parent(nullptr), failed(false)//, hash(0)
{
}

LockInspTree *LockInspTree::AddNode(void *csAddress) {
    if (this->csAddress) {
        LockInspTree newNode;

        newNode.csAddress = csAddress;
        newNode.parent = this;

        this->nodes.push_back(newNode);

        return &this->nodes.back();
    }
    else {
        this->csAddress = csAddress;
        return this;
    }
}

//// http://stackoverflow.com/questions/919612/mapping-two-integers-to-one-in-a-unique-and-deterministic-way
//void LockInspTree::AddHash(void *csAddress) {
//    // Szudzik's function
//    uint64_t a = this->hash;
//    uint64_t b = reinterpret_cast<uint64_t>(csAddress);
//    uint64_t result = a >= b ? a * a + a + b : a + b * b;
//    this->hash = result;
//}

// LockInspThreadTree=====================================================================
LockInspThreadTree::LockInspThreadTree()
    : curSubTree(nullptr), depth(0)
{
}

void LockInspThreadTree::Add(const LockInspLkItem &item) {
    if (item.lock) {
        if (this->AtRoot()) {
            this->curSubTree = &tree;
        }

        this->curSubTree = this->curSubTree->AddNode(item.csAddress);
        this->depth++;
    }
    else {
        if (this->AtRoot()) {
            this->tree.failed = true;
            this->tree.failReason += "--!--many unlocks \r\n";
            return;
        }

        if (item.csAddress != this->curSubTree->csAddress) {
            // bad locks
            this->tree.failed = true;
            this->tree.failReason += "--!--lock order not matches unlock order \r\n";
            return;
        }

        this->curSubTree = this->curSubTree->parent;
        this->depth--;
    }
}

bool LockInspThreadTree::AtRoot() const {
    return !this->curSubTree;
}

// LockInspectorImpl=====================================================================
LockInspectorImpl *LockInspectorImpl::Instance() {
    static LockInspectorImpl instance;
    return &instance;
}

void LockInspectorImpl::OnLock(void *csAddress) {
    LockInspLkItem item;

    item.threadId = std::this_thread::get_id();
    item.lock = true;
    item.csAddress = csAddress;

#ifdef LockInspectorImpl_USE_THREAD
    {
        std::unique_lock<std::mutex> lk(this->lkItemsMtx);
        this->lkItems.push_back(item);
    }

    this->lkItemsCv.notify_one();
#else
    this->ProcessItem(item);
#endif
}

void LockInspectorImpl::OnUnlock(void *csAddress) {
    LockInspLkItem item;

    item.threadId = std::this_thread::get_id();
    item.lock = false;
    item.csAddress = csAddress;

#ifdef LockInspectorImpl_USE_THREAD
    {
        std::unique_lock<std::mutex> lk(this->lkItemsMtx);
        this->lkItems.push_back(item);
    }

    this->lkItemsCv.notify_one();
#else
    this->ProcessItem(item);
#endif
}

void LockInspectorImpl::SetCurrentThreadForBreak() {
    std::unique_lock<std::mutex> lk(this->breakThreadIdMtx);
    this->breakThreadId = std::this_thread::get_id();
}

void LockInspectorImpl::ResetThreadForBreak() {
    std::unique_lock<std::mutex> lk(this->breakThreadIdMtx);
    this->breakThreadId = std::thread::id();
}

LockInspectorImpl::LockInspectorImpl()
    : exit(false)
{
#ifdef LockInspectorImpl_USE_THREAD
    this->workThread = std::thread([this]() { this->ThreadMain(); });
#endif

    this->treeThread = std::thread([this]() { this->TreeThreadMain(); });
}

LockInspectorImpl::~LockInspectorImpl() {
    {
        std::unique_lock<std::mutex> lk(this->lkItemsMtx);
        this->exit = true;
    }
    this->lkItemsCv.notify_one();

    if (this->workThread.joinable()) {
        this->workThread.join();
    }
}

void LockInspectorImpl::ThreadMain() {
    while (true) {
        {
            std::unique_lock<std::mutex> lk(this->lkItemsMtx);
            while (this->lkItems.empty() && !this->exit) {
                this->lkItemsCv.wait(lk);
            }
            this->curLkItems = this->lkItems;
            this->lkItems.clear();

            if (this->exit) {
                break;
            }
        }

        this->ProcessItems();
    }
}

void LockInspectorImpl::ProcessItems() {
    for (auto &i : this->curLkItems) {
        this->ProcessItem(i);
    }
}

void LockInspectorImpl::ProcessItem(const LockInspLkItem &item) {
    {
        std::unique_lock<std::mutex> lk(this->breakThreadIdMtx);
        if (this->breakThreadId == item.threadId) {
            int stop = 342;
        }
    }

    std::unique_lock<std::mutex> lk(this->treeMtx);
    auto threadData = &this->curThreadTree[item.threadId];
    threadData->Add(item);

    if (threadData->depth > 1000) {
        int stop = 432;
    }

    bool clear = false;

    if (threadData->tree.failed) {
        int stop = 234;
        clear = true;
    }

    if (!item.lock && threadData->AtRoot() && !threadData->tree.failed) {
        int stop = 342;

        clear = true;
    }

    if (clear) {
        if(!threadData->tree.failed) {
            {
                std::unique_lock<std::mutex> lk2(this->treeItemsMtx);
                this->treeItems.push_back(threadData->tree);
            }
            this->treeItemsCv.notify_one();
        }

        this->curThreadTree.erase(item.threadId);
    }
}

void LockInspectorImpl::TreeThreadMain() {
    while (true) {
        {
            std::unique_lock<std::mutex> lk(this->treeItemsMtx);
            while (this->treeItems.empty() && !this->exit) {
                this->treeItemsCv.wait(lk);
            }
            this->curTreeItems = this->treeItems;
            this->treeItems.clear();

            if (this->exit) {
                break;
            }
        }

        this->ProcessTreeItems();
    }
}

void LockInspectorImpl::ProcessTreeItems() {

}
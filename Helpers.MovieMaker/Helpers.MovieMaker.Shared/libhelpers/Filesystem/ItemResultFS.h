#pragma once
#include "IFileFS.h"
#include "IFolderFS.h"
#include "..\Macros.h"

#include <memory>
#include <string>

namespace Filesystem {
    template<class T> class ItemResult {
    public:
        NO_COPY(ItemResult);

        ItemResult(T *item)
            : item(item)
        {}

        ItemResult(T *item, const std::wstring &token) 
            : item(item), token(token)
        {}

        ItemResult(ItemResult &&other) 
            : item(std::move(other.item)), 
            token(std::move(other.token))
        {}

        ItemResult &operator=(ItemResult &&other) {
            if (this != &other) {
                this->item = std::move(other.item);
                this->token = std::move(other.token);
            }

            return *this;
        }

        // token - something(path, id) that can be used with IFilesystem::GetFile, IFilesystem::GetFolder methods
        bool IsTokenNew() const {
            return !this->token.empty();
        }

        // token - something(path, id) that can be used with IFilesystem::GetFile, IFilesystem::GetFolder methods
        std::wstring GetToken() const {
            return this->token;
        }

        std::unique_ptr<T> DetachItem() {
            return std::move(this->item);
        }

        const T* GetItem() {
          return this->item.get();
        }
        
        std::shared_ptr<T> GetItemShared() {
            return std::shared_ptr<T>(this->item.release());
        }

    private:
        std::unique_ptr<T> item;
        std::wstring token;
    };

    typedef ItemResult<IFile> FileResult;
    typedef ItemResult<IFolder> FolderResult;
}
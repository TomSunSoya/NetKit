//
// Created by qiuyudai on 2023/12/6.
//

#ifndef NETKIT_COPYABLE_H
#define NETKIT_COPYABLE_H

/// A tag class emphasises the objects are copyable.
/// The empty base class optimization applies.
/// Any derived class of copyable should be a value type.
class copyable {
protected:
    copyable() = default;
    ~copyable() = default;
};


#endif //NETKIT_COPYABLE_H

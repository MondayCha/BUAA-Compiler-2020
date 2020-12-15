//
// Created by dlely on 2020/12/8.
//

#include "ActiveOptimize.h"

ActiveOptimize::ActiveOptimize() = default;

ActiveOptimize &ActiveOptimize::getInstance() {
    static ActiveOptimize instance;
    return instance;
}

#pragma once

#include<string>
#include<vector>
#include"types.h"

namespace abyys {
    struct DiffLine {
        DiffLineType type;
        std::string content;
        int old_line_no;
        int new_line_no;
    };
    struct DiffHunk
    {
        int old_start;
        int new_start;
        std::vector<DiffLine> lines;
        
    };

    struct DiffResult {
        bool too_large = false;
        bool is_binary = false;
        std::string message;
        std::vector<DiffHunk> hunks;
    };
    
    
}

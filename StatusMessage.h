//
// Created by mac on 5/8/16.
//

#ifndef FINALPROJECTBIOMED_STATUSMESSAGE_H
#define FINALPROJECTBIOMED_STATUSMESSAGE_H

// helper class to format slice status message
class StatusMessage {
public:
    static std::string Format(int slice, int maxSlice) {
        std::stringstream tmp;
        tmp << "Slice Number  " << slice + 1 << "/" << maxSlice + 1;
        return tmp.str();
    }
};

#endif //FINALPROJECTBIOMED_STATUSMESSAGE_H

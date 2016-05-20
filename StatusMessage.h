/*!
 * @brief Helper to display text on axial view UI
 * @file
 * */
#ifndef FINALPROJECTBIOMED_STATUSMESSAGE_H
#define FINALPROJECTBIOMED_STATUSMESSAGE_H

//! A helper class to display slice status messages on the axial view
class StatusMessage {
public:
    //! Returns the Formated slice string
    static std::string Format(int slice, int maxSlice) {
        std::stringstream tmp;
        tmp << "Slice Number  " << slice + 1 << "/" << maxSlice + 1;
        return tmp.str();
    }
};

#endif //FINALPROJECTBIOMED_STATUSMESSAGE_H

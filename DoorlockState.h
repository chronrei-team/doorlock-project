#ifndef _DOORLOCKSTATE_H_
#define _DOORLOCKSTATE_H_

enum class DoorlockState {
    Open, // 열림
    Close, // 닫힘
    InputOnClose, // 비밀번호 입력
    PasswordFail, // 비밀번호 실패
    OpenAction, // 열리는 중
    CloseAction, // 닫히는 중
};

extern DoorlockState doorlockState;

#endif
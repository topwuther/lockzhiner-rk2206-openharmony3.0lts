/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <setting_wifi_input_password_ability_slice.h>
#include "gfx_utils/style.h"

namespace OHOS {
REGISTER_AS(SettingWifiInputPasswordAbilitySlice)

static UIView::OnClickListener* clickLeftListener_ = nullptr;

static const int g_maxPassword = 10;    // Maximum length of a password.
static char* g_inputSsid = nullptr;
static char g_inputPassword[g_maxPassword + 1] = { 0 };
static int g_inputCount = 0;
static int g_cursorPositionX = 20;    // Initial position of cursor X

SettingWifiInputPasswordAbilitySlice::~SettingWifiInputPasswordAbilitySlice()
{
    if (scrollView_) {
        DeleteChildren(scrollView_);
        scrollView_ = nullptr;
    }
    if (inputView_) {
        DeleteChildren(inputView_);
        inputView_ = nullptr;
    }
    if (headView_) {
        DeleteChildren(headView_);
        headView_ = nullptr;
    };
    if (buttonBackListener_) {
        delete buttonBackListener_;
        buttonBackListener_ = nullptr;
    }
    if (clickLeftListener_) {
        delete clickLeftListener_;
        clickLeftListener_ = nullptr;
    }
}

class TestBtnOnClickInputPasswordChangeListener : public OHOS::UIView::OnClickListener {
public:
    ~TestBtnOnClickInputPasswordChangeListener() {}
    TestBtnOnClickInputPasswordChangeListener(UILabel* uiLabel, UILabel* uiCursor, const int ii,
        const int cursorOffset) : myUiLabel(uiLabel), myUiCursor(uiCursor), mIi(ii), myCursorOffset(cursorOffset) {}
    bool OnClick(UIView& view, const ClickEvent& event) override
    {
        if (g_inputCount >= g_maxPassword) {
            return true;
        }
        g_inputPassword[g_inputCount] = '0' + mIi;
        myUiLabel->SetText(g_inputPassword);
        g_inputCount++;
        g_inputPassword[g_inputCount] = '\0';
        g_cursorPositionX += myCursorOffset;
        myUiCursor->SetX(g_cursorPositionX);
        return true;
    }

private:
    UILabel* myUiLabel;
    UILabel* myUiCursor;
    int mIi;
    int myCursorOffset;
};

class TestBtnOnClickEnterChangeListener : public OHOS::UIView::OnClickListener {
public:
    explicit TestBtnOnClickEnterChangeListener(SettingWifiInputPasswordAbilitySlice* slice) : mySlice(slice) {}
    ~TestBtnOnClickEnterChangeListener() {}
    bool OnClick(UIView& view, const ClickEvent& event) override
    {
        WpaScanReconnect(g_inputSsid, g_inputPassword, HIDDEN_OPEN);
        g_inputSsid = nullptr;
        int err = memset_s(g_inputPassword, sizeof(g_inputPassword), 0, sizeof(g_inputPassword));
        if (err != EOK) {
            printf("[ERROR]memset_s failed, err = %d\n", err);
            return false;
        }
        mySlice->Terminate();
        return true;
    }
    SettingWifiInputPasswordAbilitySlice* mySlice;
};

void SettingWifiInputPasswordAbilitySlice::SetButtonListener(void)
{
    auto onClick = [this](UIView& view, const Event& event) -> bool {
        Terminate();
        return true;
    };
    buttonBackListener_ = new EventListener(onClick, nullptr);
}

void SettingWifiInputPasswordAbilitySlice::SetHead(void)
{
    headView_ = new UIViewGroup();
    rootView_->Add(headView_);
    headView_->SetPosition(DE_HEAD_X, DE_HEAD_Y, DE_HEAD_WIDTH, DE_HEAD_HEIGHT);
    headView_->SetStyle(STYLE_BACKGROUND_OPA, 0);
    headView_->SetTouchable(true);
    headView_->SetOnClickListener(buttonBackListener_);

    UIImageView* imageView = new UIImageView();
    headView_->Add(imageView);
    imageView->SetPosition(DE_HEAD_IMAGE_X, DE_HEAD_IMAGE_Y, DE_HEAD_IMAGE_WIDTH, DE_HEAD_IMAGE_HEIGHT);
    imageView->SetSrc(DE_IMAGE_BACK);

    UILabel* lablelFont = new UILabel();
    lablelFont->SetPosition(DE_HEAD_TEXT_X, DE_HEAD_TEXT_Y, DE_HEAD_TEXT_WIDTH, DE_HEAD_TEXT_HEIGHT);
    lablelFont->SetText(g_inputSsid);
    lablelFont->SetFont(DE_FONT_OTF, DE_HEAD_TEXT_SIZE);
    lablelFont->SetStyle(STYLE_TEXT_COLOR, DE_HEAD_TEXT_COLOR);
    headView_->Add(lablelFont);
}

void SettingWifiInputPasswordAbilitySlice::SetInput(void)
{
    inputView_ = new UIViewGroup();
    inputView_->SetPosition(INPUT_X, INPUT_Y, INPUT_WIDTH, INPUT_HEIGHT);

    inputView_->SetStyle(STYLE_BACKGROUND_COLOR, DE_BUTTON_BACKGROUND_COLOR);
    inputView_->SetStyle(STYLE_BACKGROUND_OPA, DE_OPACITY_ALL);
    inputView_->SetStyle(STYLE_BORDER_RADIUS, DE_BUTTON_RADIUS);
    rootView_->Add(inputView_);

    lablelInputText_ = new UILabel();
    lablelInputText_->SetPosition(INPUT_TEXT_X, INPUT_TEXT_Y, INPUT_TEXT_WIDTH, INPUT_TEXT_HEIGHT);
    lablelInputText_->SetStyle(STYLE_BACKGROUND_COLOR, DE_BUTTON_BACKGROUND_COLOR);
    lablelInputText_->SetStyle(STYLE_BACKGROUND_OPA, DE_OPACITY_ALL);
    lablelInputText_->SetText("输入密码");
    lablelInputText_->SetFont(DE_FONT_OTF, DE_TITLE_TEXT_SIZE);
    inputView_->Add(lablelInputText_);

    lablelCursorText_ = new UILabel();
    lablelCursorText_->SetPosition(g_cursorPositionX, INPUT_CURSOR_Y, INPUT_CURSOR_WIDTH, INPUT_CURSOR_HEIGHT);
    lablelCursorText_->SetStyle(STYLE_BACKGROUND_COLOR, Color::ColorTo32(Color::GetColorFromRGB(0x0D, 0x9F, 0xF8)));
    lablelCursorText_->SetStyle(STYLE_BACKGROUND_OPA, DE_OPACITY_ALL);
    inputView_->Add(lablelCursorText_);

    UIViewGroup* enterView = new UIViewGroup();
    enterView->SetPosition(INPUT_ENTER_X, INPUT_ENTER_Y, INPUT_ENTER_WIDTH, INPUT_ENTER_HEIGHT);
    enterView->SetStyle(STYLE_BACKGROUND_COLOR, Color::ColorTo32(Color::GetColorFromRGB(0x0D, 0x9F, 0xF8)));
    enterView->SetStyle(STYLE_BACKGROUND_OPA, DE_OPACITY_ALL);
    enterView->SetStyle(STYLE_BORDER_RADIUS, DE_BUTTON_RADIUS);
    inputView_->Add(enterView);

    UIImageView* imageView = new UIImageView();
    imageView->SetPosition(INPUT_IMAGE_X, INPUT_IMAGE_Y, INPUT_IMAGE_WIDTH, INPUT_IMAGE_HEIGHT);
    imageView->SetSrc(DE_IMAGE_ENTER);
    enterView->Add(imageView);
    imageView->SetTouchable(true);
    clickLeftListener_ = new TestBtnOnClickEnterChangeListener(this);
    imageView->SetOnClickListener(clickLeftListener_);
}

void SettingWifiInputPasswordAbilitySlice::AddInputKeyBoardZero(void)
{
    char buf[8] = {0};
    int myUseX = BUTTON_INTERVAL_X;
    int myUseY = 198;
    int inputNum = 0;
    UILabelButton* inputButton = new UILabelButton();

    inputButton->SetPosition(myUseX, myUseY);
    sprintf_s(buf, sizeof(buf), "%d", inputNum);
    inputButton->SetWidth(BUTTON_WIDTH);
    inputButton->SetHeight(BUTTON_HEIGHT);
    inputButton->SetText(buf);
    inputButton->SetStyle(STYLE_BACKGROUND_COLOR, DE_BUTTON_BACKGROUND_COLOR);
    inputButton->SetStyle(STYLE_BORDER_RADIUS, RECT_RADIUS);
    inputButton->SetStyle(STYLE_TEXT_COLOR, DE_TITLE_TEXT_COLOR);
    inputButton->SetFont(DE_FONT_OTF, DE_TITLE_TEXT_SIZE);

    clickLeftListener_ = new TestBtnOnClickInputPasswordChangeListener((UILabel*)lablelInputText_, (UILabel*)lablelCursorText_, inputNum, CURSOR_POSITION_OFFSET);
    inputButton->SetOnClickListener(clickLeftListener_);
    scrollView_->Add(inputButton);
}

void SettingWifiInputPasswordAbilitySlice::SetScrollView(void)
{
    char buf[8] = {0};
    int inputNum;
    scrollView_ = new UIScrollView();
    scrollView_->SetPosition(SCROLL_WIFI_INPUT_X, SCROLL_WIFI_INPUT_Y, SCROLL_WIFI_INPUT_WIDTH,
        SCROLL_WIFI_INPUT_WIDTH);
    scrollView_->SetStyle(STYLE_BACKGROUND_COLOR, DE_SCROLL_COLOR);
    scrollView_->SetXScrollBarVisible(false);
    scrollView_->SetYScrollBarVisible(false);
    rootView_->Add(scrollView_);
    for (int countFirst = 0; countFirst < BUTTON_NUM; countFirst++) {
        for (int countSecound = 0; countSecound < BUTTON_NUM; countSecound++) {
            int myUseX = countSecound * BUTTON_INTERVAL_X;
            int myUseY = countFirst * BUTTON_INTERVAL_Y;
            inputNum = ((countFirst * BUTTON_NUM) + countSecound + 1);
            UILabelButton* inputButton = new UILabelButton();
            inputButton->SetPosition(myUseX, myUseY, BUTTON_WIDTH, BUTTON_HEIGHT);
            int err = sprintf_s(buf, sizeof(buf), "%d", inputNum);
            if (err < 0) {
                printf("[ERROR]sprintf_s failed, err = %d\n", err);
                return;
            }
            inputButton->SetText(buf); // SetText is system functions
            inputButton->SetStyle(STYLE_BACKGROUND_COLOR, DE_BUTTON_BACKGROUND_COLOR);
            inputButton->SetStyle(STYLE_BORDER_RADIUS, RECT_RADIUS);
            inputButton->SetStyle(STYLE_TEXT_COLOR, DE_TITLE_TEXT_COLOR);
            inputButton->SetFont(DE_FONT_OTF, DE_CONTENT_FONT_SIZE);
            UIView::OnClickListener* clickLeftListener = nullptr;
            clickLeftListener = new TestBtnOnClickInputPasswordChangeListener((UILabel*)lablelInputText_,
                (UILabel*)lablelCursorText_, inputNum, CURSOR_POSITION_OFFSET);
            inputButton->SetOnClickListener(clickLeftListener);
            scrollView_->Add(inputButton);
        }
    }
    AddInputKeyBoardZero();
}

void SettingWifiInputPasswordAbilitySlice::OnStart(const Want& want)
{
    printf("[LOG]receive the data -> %s\n", reinterpret_cast<char*>(want.data));
    AbilitySlice::OnStart(want);

    g_inputSsid = reinterpret_cast<char*>(want.data);
    rootView_ = RootView::GetWindowRootView();

    int err = memset_s(g_inputPassword, sizeof(g_inputPassword), 0, g_maxPassword);
    if (err != EOK) {
        printf("[ERROR]memcpy_s failed, err = %d\n", err);
        return;
    }
    g_inputCount = 0;
    g_cursorPositionX = 20; // 20
    rootView_->SetPosition(DE_ROOT_X, DE_ROOT_Y, DE_ROOT_WIDTH, DE_ROOT_HEIGHT);
    rootView_->SetStyle(STYLE_BACKGROUND_COLOR, DE_ROOT_BACKGROUND_COLOR);
    SetButtonListener();
    SetHead();
    SetInput();
    SetScrollView();
    SetUIContent(rootView_);
}

void SettingWifiInputPasswordAbilitySlice::OnInactive()
{
    AbilitySlice::OnInactive();
}

void SettingWifiInputPasswordAbilitySlice::OnActive(const Want& want)
{
    int err;
    lablelInputText_->SetText("输入密码");
    g_cursorPositionX = 20; // 20
    lablelCursorText_->SetX(g_cursorPositionX);
    g_inputCount = 0;
    err = memset_s(g_inputPassword, sizeof(g_inputPassword), 0, sizeof(g_inputPassword));
    if (err != EOK) {
        return;
    }
    AbilitySlice::OnActive(want);
}

void SettingWifiInputPasswordAbilitySlice::OnBackground()
{
    AbilitySlice::OnBackground();
}

void SettingWifiInputPasswordAbilitySlice::OnStop()
{
    AbilitySlice::OnStop();
}
} // namespace OHOS

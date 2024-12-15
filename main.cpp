#include <Application.h>
#include <Window.h>
#include <View.h>
#include <StringView.h>
#include <TextControl.h>
#include <Button.h>
#include <Alert.h>
#include <String.h>
#include <Handler.h>
#include <Looper.h>
#include <MessageRunner.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Variables to store game state
int timeLeft = 90;           // Countdown timer starts at 90 seconds.
int score = 0;               // Player's score starts at 0.
bool timerPaused = true;     // Timer starts paused.
BString currentColor;        // The correct color name for the round.
const char* colorNames[] = {"Red", "Blue", "Green", "Yellow", "Pink", "Orange", "Black", "Gray"};
const char* colorHex[] = {"#FF0000", "#0000FF", "#00FF00", "#FFFF00", "#FFC0CB", "#FFA500", "#000000", "#808080"};

// Declare pointers for UI elements
BStringView* timeLabel;      // Label to display the time left.
BStringView* scoreLabel;     // Label to display the player's score.
BStringView* colorLabel;     // Label to show the color name.
BTextControl* textEntry;     // Input field for the player's guess.
BButton* restartButton;      // Button to restart the game.
BMessageRunner* timerRunner; // Timer message runner to update the countdown.

// Function to update the timer every second
void UpdateTimer() {
    if (!timerPaused && timeLeft > 0) {
        timeLeft--; // Decrease the time by 1 second.

        // Update the time label to show the remaining time.
        BString timeText;
        timeText.SetToFormat("Time left: %d", timeLeft);
        timeLabel->SetText(timeText.String());

        // If time runs out, pause the timer and end the game
        if (timeLeft == 0) {
            timeLabel->SetText("Game Over!");
            textEntry->SetEnabled(false); // Disable input
            timerPaused = true;
        }
    }
}

// Function to update the game to the next color
void NextColor() {
    // Pick random indices for the text and the color
    int textIndex = rand() % 8;  // Random text color name
    int colorIndex = rand() % 8; // Random actual color

    // Set the current color for comparison
    currentColor.SetTo(colorNames[colorIndex]);

    // Update the colorLabel to display the text color name
    colorLabel->SetText(colorNames[textIndex]);

    // Change the color label's color dynamically
    rgb_color rgb;
    sscanf(colorHex[colorIndex], "#%02hhX%02hhX%02hhX", &rgb.red, &rgb.green, &rgb.blue);
    colorLabel->SetHighColor(rgb.red, rgb.green, rgb.blue);
    colorLabel->Invalidate(); // Redraw with the new color.
}

// Function to check the player's input
void OnTextEntered() {
    timerPaused = false; // Start the timer when the first guess is entered.

    const char* input = textEntry->Text(); // Get the player's guess.

    if (currentColor.Compare(input) == 0) {
        score++; // Increase the score if correct.
    } else {
        score = (score > 0) ? score - 1 : 0; // Decrease the score but don't go below 0.
    }

    // Update the score label
    BString scoreText;
    scoreText.SetToFormat("Score: %d", score);
    scoreLabel->SetText(scoreText.String());

    // Clear the text entry and load the next color
    textEntry->SetText("");
    NextColor();
}

// Function to restart the game
void RestartGame() {
    timeLeft = 90;    // Reset the time.
    score = 0;        // Reset the score.
    timerPaused = true; // Pause the timer until Enter is pressed.
    textEntry->SetEnabled(true); // Re-enable text entry.

    // Update labels
    timeLabel->SetText("Time left: 90");
    scoreLabel->SetText("Score: 0");

    NextColor(); // Start a new game immediately.
}

// Custom window class for the game
class ColorGameWindow : public BWindow {
public:
    ColorGameWindow()
        : BWindow(BRect(100, 100, 500, 400), "The Color Game For Haiku OS", B_TITLED_WINDOW, B_NOT_RESIZABLE, B_QUIT_ON_WINDOW_CLOSE) {
        // Main view container
        BView* mainView = new BView(Bounds(), "MainView", B_FOLLOW_ALL, B_WILL_DRAW);
        mainView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR)); // Set background color.
        AddChild(mainView);

        // Title label
        BStringView* titleLabel = new BStringView(BRect(10, 10, 400, 50), "TitleLabel", "The Color Game For Haiku OS:");
        titleLabel->SetFont(be_bold_font);
        titleLabel->SetFontSize(20); // Bigger font size for the title.
      
        mainView->AddChild(titleLabel);

        // Time label
        timeLabel = new BStringView(BRect(10, 70, 400, 100), "TimeLabel", "Time left: 90");
        timeLabel->SetFont(be_plain_font);
        timeLabel->SetFontSize(20); // Bigger font size for the time.
        mainView->AddChild(timeLabel);

        // Score label
        scoreLabel = new BStringView(BRect(10, 110, 400, 140), "ScoreLabel", "Score: 0");
        scoreLabel->SetFont(be_plain_font);
        scoreLabel->SetFontSize(20); // Bigger font size for the score.
        mainView->AddChild(scoreLabel);

        // Color label
        colorLabel = new BStringView(BRect(10, 150, 400, 180), "ColorLabel", "Type the color!");
        colorLabel->SetFont(be_plain_font);
        colorLabel->SetFontSize(28); // Bigger font size for the color label.
        mainView->AddChild(colorLabel);

        // Text entry field (no label, bigger size)
        textEntry = new BTextControl(BRect(10, 200, 400, 260), "TextEntry", NULL, "", new BMessage('ENTR'));
        textEntry->SetDivider(0); // Remove label divider.
        mainView->AddChild(textEntry);

        // Restart button
        restartButton = new BButton(BRect(10, 230, 100, 300), "RestartButton", "Restart", new BMessage('RSTR'));
        mainView->AddChild(restartButton);

        // Set up the timer runner
        timerRunner = new BMessageRunner(BMessenger(this), new BMessage('TIME'), 1000000); // 1-second interval.

        // Display the first color immediately
        NextColor();
    }

    void MessageReceived(BMessage* msg) override {
        switch (msg->what) {
            case 'ENTR':
                OnTextEntered();
                break;
            case 'RSTR':
                RestartGame();
                break;
            case 'TIME':
                UpdateTimer();
                break;
            default:
                BWindow::MessageReceived(msg);
                break;
        }
    }
};

// Application class to manage the game
class ColorGameApp : public BApplication {
public:
    ColorGameApp() : BApplication("application/x-vnd.ColorGame") {}

    void ReadyToRun() override {
        BWindow* window = new ColorGameWindow();
        window->Show();
    }
};

int main() {
    srand(time(0)); // Seed the random number generator.
    ColorGameApp app; // Create the application instance.
    app.Run();       // Run the application.
    return 0;
}


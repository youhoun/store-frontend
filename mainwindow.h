#pragma once

#include <QMainWindow>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QSet>
#include <QByteArray>
#include <QString>

class QStackedWidget;
class QListWidget;
class QLineEdit;
class QLabel;
class QComboBox;
class QTableWidget;
class QPlainTextEdit;
class QGroupBox;
class QPushButton;
class QTcpServer;
class QNetworkAccessManager;
class QNetworkReply;
class QDialog;
class QTimer;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    struct Lecture {
        QString title;
        QString teacher;
        QString category;
        QString path;
        bool free = true;
    };

private slots:
    void showHome();
    void showLectures();
    void showMyBooks();
    void showAdmin();
    void showLogin();
    void logout();
    void startGoogleLogin();
    void handleGoogleConnection();
    void exchangeGoogleCode(const QString &code);
    void finishGoogleLogin(const QByteArray &json);
    void searchBooks();
    void filterBooks(int index);
    void searchPaidLectures();
    void pollPaymentStatus();
    void updatePaymentCountdown();
    void sendHeartbeat();
    void showLectureRatingDialog(const Lecture &lecture);
    void refreshLectureRating(const Lecture &lecture, QLabel *ratingLabel);
    void addAdminItem();

private:
    struct Book {
        QString id, title, author, category, description, content;
        bool free = true;
        double price = 0.0;
    };

    void loadData();
    void buildUi();
    QWidget *makeHeader();
    QWidget *makeHomePage();
    QWidget *makeLecturesPage();
    QWidget *makeMyBooksPage();
    QWidget *makeAdminPage();
    QWidget *makeLoginPage();
    QWidget *bookCard(const Book &book);
    QWidget *lectureCard(const Lecture &lecture);
    void refreshHome();
    void refreshLectures();
    void refreshMyBooks();
    void fetchOwnedLectures();
    void updateUserLabel();
    void notifyLogin();
    void refreshAdminStats();
    QString assetPath(const QString &relative) const;
    void showBookDialog(const Book &book);
    void showLectureDialog(const Lecture &lecture);
    void openPdf(const Lecture &lecture);
    Book *findBook(const QString &id);
    void startLecturePayment(const Lecture &lecture);
    void closePaymentDialog(bool unlocked);

    QStackedWidget *pages = nullptr;
    QListWidget *navigation = nullptr;
    QLineEdit *searchEdit = nullptr;
    QLineEdit *paidLectureSearchEdit = nullptr;
    QComboBox *filterCombo = nullptr;
    QLabel *userLabel = nullptr;

    QWidget *homeContainer = nullptr;
    QWidget *lectureHomeContainer = nullptr;
    QWidget *lectureContainer = nullptr;
    QWidget *myBooksContainer = nullptr;

    // Lecture ids (their asset path) the signed-in user has already
    // paid for. Populated from the backend on login via
    // GET /api/purchases/:email so a paid lecture can never be
    // "bought" twice — the app just opens it straight away.
    QSet<QString> ownedLectures;

    QTableWidget *adminUsersTable = nullptr;
    QLabel *adminUsersCountValue = nullptr;
    QLabel *adminOnlineCountValue = nullptr;
    QLabel *adminBooksBoughtValue = nullptr;
    QLabel *adminRevenueValue = nullptr;
    QTimer *heartbeatTimer = nullptr;
    QList<Book> books;
    QList<Lecture> lectures;

    QString currentUserName;
    QString currentUserEmail;
    bool admin = false;

    QTcpServer *oauthServer = nullptr;
    QNetworkAccessManager *networkManager = nullptr;
    QString oauthState;
    QString oauthCodeVerifier;
    QString oauthRedirectUri;

    // Payment state (Tola Saint, proxied through our own backend).
    QDialog *paymentDialog = nullptr;
    QLabel *paymentStatusLabel = nullptr;
    QTimer *paymentPollTimer = nullptr;
    QTimer *paymentCountdownTimer = nullptr;
    QString paymentId;
    Lecture paymentLecture;
    qint64 paymentExpiresAtMs = 0;
    QLabel *paymentCountdownLabel = nullptr;
};

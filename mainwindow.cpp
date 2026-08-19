#include "mainwindow.h"

#include <QApplication>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFrame>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPixmap>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSpacerItem>
#include <QStackedWidget>
#include <QStyle>
#include <QTableWidget>
#include <QAbstractItemView>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QTextBrowser>
#include <QTextStream>
#include <QVBoxLayout>
#include <QDesktopServices>
#include <QUrl>
#include <QUrlQuery>
#include <QRandomGenerator>
#include <QCryptographicHash>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QCoreApplication>
#include <QTimer>
#include <QJsonParseError>
#include <QJsonArray>
#include <QDateTime>
#include <QColor>
#include <QUuid>

namespace {
const QString INK = "#1B2430";
const QString INK_SOFT = "#3B4657";
const QString PARCHMENT = "#FAF6EC";
const QString LINE = "#E3DAC4";
const QString SAFFRON = "#E4A335";
const QString TEAL = "#2F7A6F";

// Paste the OAuth 2.0 Client ID from Google Cloud here.
// Create it as an application type: Desktop app.
const QString GOOGLE_CLIENT_ID =
    "......";

const QString GOOGLE_CLIENT_SECRET =
    "GOCSPX-......";

// URL of the small backend that holds the Tola Saint secret key and
// creates/checks payments on our behalf. Point this at your deployed
// Render service (see sabai-backend/README.md), no trailing slash.
const QString PAYMENTS_BACKEND_URL =
    "https://store-backend-ap0b.onrender.com";

// Only this account can see/use the Admin area. Must match ADMIN_EMAIL
// in the backend's server.js (that's the copy that actually gates the
// admin stats endpoint — this one just controls what the UI shows).
const QString ADMIN_EMAIL = "youhounx@gmail.com";

// How often the app pings the backend while signed in, so the admin
// dashboard can tell who's currently online. Keep this comfortably
// below the backend's ONLINE_WINDOW_MS (90s) in store.js.
const int HEARTBEAT_INTERVAL_MS = 30 * 1000;
}
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    resize(1180, 760);
    setMinimumSize(900, 620);
    loadData();
    buildUi();
    networkManager = new QNetworkAccessManager(this);
    oauthServer = new QTcpServer(this);
    connect(oauthServer, &QTcpServer::newConnection, this, &MainWindow::handleGoogleConnection);

    heartbeatTimer = new QTimer(this);
    heartbeatTimer->setInterval(HEARTBEAT_INTERVAL_MS);
    connect(heartbeatTimer, &QTimer::timeout, this, &MainWindow::sendHeartbeat);

    showHome();
}

QString MainWindow::assetPath(const QString &relative) const
{
    // During development, assets are next to the executable.
    // Copy the "assets" folder beside the .exe after building.
    return QDir(QCoreApplication::applicationDirPath()).filePath("assets/" + relative);
}

void MainWindow::loadData()
{
    QFile f(":/data.json");

    if (f.open(QIODevice::ReadOnly)) {
        const auto doc = QJsonDocument::fromJson(f.readAll());
        const auto root = doc.object();

        for (const auto v : root.value("books").toArray()) {
            const auto o = v.toObject();

            Book b;
            b.id = o.value("id").toString();
            b.title = o.value("title").toString();
            b.author = o.value("author").toString();
            b.category = o.value("category").toString();
            b.description = o.value("description").toString();
            b.content = o.value("content").toString();
            b.free = o.value("isFree").toBool();
            b.price = o.value("price").toDouble();

            books.append(b);
        }
    }


    // Lecture metadata mirrors the original web project's lecture folders.
    const QStringList freeLectures = {
        "free/M.Theara C++/CP note books.pdf",
        "free/M.Theara C++/CP - Lecture 4 - Values and Variables - 2.pdf",
        "free/M.Theara C++/CP-W2-Getting Started with C++1.pdf",
        "free/M.Theara C++/Computer Programming with C++.pdf",
        "free/M.Theara C++/Chapter 03-Lab-5-6.pdf",
        "free/M.Theara C++/Ch02-Lab.pdf",
        "free/M.Theara C++/CP-W1-Introduction to CP.pdf",
        "free/M.Theara C++/CP - Lecture 6 - Selection.pdf",
        "free/M.Theara C++/CP - Lecture 7-While Loop.pdf",
        "free/M.Theara C++/CP - Lecture 5 - Values and Variables - 3.pdf",
        "free/M.Theara C++/Chapter 03-Lab-3-4.pdf",
        "free/M.Theara C++/CP - Lecture 3 - Values and Variables - 1.pdf",
        "free/M.Theara C++/Chapter 03-Lab-1-2.pdf",
        "free/Programming Methodology in C++/21-Inheritance_c++.pdf",
        "free/Programming Methodology in C++/1-Essential_C++_SLO and Objective.pdf",
        "free/Programming Methodology in C++/20.FriendShip_inheritance.pdf",
        "free/Programming Methodology in C++/5-Essential_C++_Operator.pdf",
        "free/Programming Methodology in C++/23.function_template_class_teamplate.pdf",
        "free/Programming Methodology in C++/19.Overloading_Static Member_Part2_C++.pdf",
        "free/Programming Methodology in C++/2-Essential_C++_Install.pdf",
        "free/Programming Methodology in C++/22_Abstraction_c++.pdf",
        "free/Programming Methodology in C++/18.Structure_union_C++.pdf",
        "free/Programming Methodology in C++/17.Pointer_To_Class_C++.pdf",
        "free/Programming Methodology in C++/16.Constructor_Destructor_c++.pdf",
        "free/Programming Methodology in C++/15.Class_Object_c++.pdf",
        "free/Ms.Lyda C++/12_Map.pdf",
        "free/Ms.Lyda C++/09_Character & String.pdf",
        "free/Ms.Lyda C++/07_Multi-dimensional Arrays.pdf",
        "free/Ms.Lyda C++/11_Vector.pdf",
        "free/Ms.Lyda C++/10_User-defined Function.pdf",
        "free/Ms.Lyda C++/08_Jump Statements.pdf",
        "free/Ms.Lyda C++/06_Introduction to C++.pdf"
    };

    const QStringList paidLectures = {
        "paid/Python Programming - CodeKhmerLearning.pdf",
        "paid/c_programming.pdf",
        "paid/Git Notes for Professionals.pdf",
        "paid/Linux commands Notes for Professionals.pdf",
        "paid/PHP Notes for Professionals.pdf",
        "paid/Node.js Notes for Professionals.pdf",
        "paid/JavaScript - CodeKhmerLearning.pdf",
        "paid/Learning MongoDB.pdf",
        "paid/PostgreSQL Notes for Professionals.pdf",
        "paid/Robotics, AI, and Humanity.pdf",
        "paid/PHP - CodeKhmerLearning.pdf",
        "paid/ReactJS - CodeKhmerLearning.pdf"
    };

    auto addLectures = [this](const QStringList &items, bool isFree) {

        for (const QString &path : items) {

            Lecture l;

            l.path = path;
            l.free = isFree;

            const QString file =
                QFileInfo(path).completeBaseName();

            l.title = file;

            l.teacher =
                path.startsWith("free/M.Theara")
                    ? "M. Theara"
                    : path.startsWith("free/Ms.Lyda")
                          ? "Ms. Lyda"
                          : path.startsWith("free/Programming")
                                ? "Programming Methodology"
                                : "Sabai Books";

            l.category = "C++ / Programming";

            lectures.append(l);
        }
    };

    addLectures(freeLectures, true);
    addLectures(paidLectures, false);

    // Admin-created content is stored outside the Qt resource file so it
    // survives restarts and can be managed from the Admin page.
    QFile custom(QCoreApplication::applicationDirPath() + "/admin_catalog.json");
    if (custom.open(QIODevice::ReadOnly)) {
        const auto arr = QJsonDocument::fromJson(custom.readAll()).array();
        for (const auto &v : arr) {
            const auto o = v.toObject();
            const QString type = o.value("type").toString();
            if (type == "lecture") {
                Lecture l; l.path=o.value("path").toString(); l.title=o.value("title").toString();
                l.teacher=o.value("author").toString(); l.category=o.value("category").toString(); l.free=o.value("price").toDouble()<=0;
                if (!l.path.isEmpty() && !l.title.isEmpty()) lectures.append(l);
            } else {
                Book b; b.id=o.value("id").toString(); b.title=o.value("title").toString(); b.author=o.value("author").toString();
                b.category=o.value("category").toString(); b.price=o.value("price").toDouble(); b.free=b.price<=0; b.content=o.value("content").toString();
                if (!b.title.isEmpty()) books.append(b);
            }
        }
    }
}

QWidget *MainWindow::makeHeader()
{
    auto *header = new QWidget;
    header->setObjectName("header");
    auto *layout = new QHBoxLayout(header);
    layout->setContentsMargins(22, 12, 22, 12);

    auto *brand = new QPushButton("📖  Sabai Books");
    brand->setObjectName("brand");
    brand->setCursor(Qt::PointingHandCursor);
    connect(brand, &QPushButton::clicked, this, &MainWindow::showHome);

    userLabel = new QLabel;
    userLabel->setObjectName("userLabel");

    auto *login = new QPushButton;
    login->setObjectName("headerButton");
    connect(login, &QPushButton::clicked, this, [this, login] {
        if (currentUserName.isEmpty()) showLogin();
        else logout();
    });

    layout->addWidget(brand);
    layout->addStretch();
    layout->addWidget(userLabel);
    layout->addWidget(login);
    updateUserLabel();

    return header;
}

void MainWindow::buildUi()
{
    auto *root = new QWidget;
    auto *outer = new QVBoxLayout(root);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);
    outer->addWidget(makeHeader());

    auto *body = new QWidget;
    auto *bodyLayout = new QHBoxLayout(body);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(0);

    navigation = new QListWidget;
    navigation->setObjectName("navigation");
    navigation->addItem("🏠  Home");
    navigation->addItem("💰  Paid Lectures");
    navigation->addItem("🔖  My Books");
    navigation->addItem("⚙  Admin");
    navigation->setFixedWidth(190);
    navigation->setRowHidden(3, !admin); // hidden until an admin signs in
    connect(navigation, &QListWidget::currentRowChanged, this, [this](int row) {
        if (row == 0) showHome();
        else if (row == 1) showLectures();
        else if (row == 2) showMyBooks();
        else showAdmin();
    });

    pages = new QStackedWidget;
    pages->addWidget(makeHomePage());
    pages->addWidget(makeLecturesPage());
    pages->addWidget(makeMyBooksPage());
    pages->addWidget(makeAdminPage());
    pages->addWidget(makeLoginPage());

    bodyLayout->addWidget(navigation);
    bodyLayout->addWidget(pages, 1);
    outer->addWidget(body, 1);

    setCentralWidget(root);

    setStyleSheet(QString(R"(
        QMainWindow, QWidget { background: %1; color: %2; font-family: "Segoe UI"; font-size: 14px; }
        #header { background: %2; color: %3; }
        #brand { background: transparent; color: %3; border: none; font-size: 21px; font-weight: 700; }
    #userLabel {
        background: transparent;
        color: %3;
        padding-right: 12px;
    }
        #headerButton { background: %4; color: %2; border: none; border-radius: 8px; padding: 8px 14px; font-weight: 700; }
        #navigation { background: #F1EBDC; border: none; padding: 14px 8px; outline: none; }
        #navigation::item { padding: 13px 12px; margin: 3px 0; border-radius: 8px; }
        #navigation::item:selected { background: %2; color: %3; }
        #navigation::item:hover { background: #E7DECB; }
        QLineEdit, QComboBox, QPlainTextEdit, QTextBrowser { background: white; border: 1px solid %5; border-radius: 8px; padding: 9px; }
        QPushButton { border: 1px solid %5; border-radius: 8px; padding: 9px 14px; background: white; }
        QPushButton:hover { border-color: %2; }
        .primary { background: %4; color: %2; border: none; font-weight: 700; }
        .teal { background: %6; color: white; border: none; font-weight: 700; }
        .card { background: white; border: 1px solid %5; border-radius: 12px; }
        QLabel#pageTitle { font-size: 30px; font-weight: 700; }
        QLabel#muted { color: %7; }
        QLabel#bookTitle { font-size: 17px; font-weight: 700; }
        QLabel#tag { background: #E4F1EC; color: %6; border-radius: 10px; padding: 4px 8px; font-weight: 700; }
        QHeaderView::section { background: #F1EBDC; padding: 8px; border: none; font-weight: 700; }
        QTableWidget { background: white; border: 1px solid %5; border-radius: 8px; gridline-color: %5; }
    )").arg(PARCHMENT, INK, PARCHMENT, SAFFRON, LINE, TEAL, INK_SOFT));

    navigation->setCurrentRow(0);
}

QWidget *MainWindow::makeHomePage()
{
    auto *page = new QWidget;

    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);

    auto *title = new QLabel("Free Lectures");
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    auto *subtitle = new QLabel(
        "Learn C++ and programming with our free lecture materials."
        );
    subtitle->setObjectName("muted");
    layout->addWidget(subtitle);

    layout->addSpacing(16);

    // Search lectures
    searchEdit = new QLineEdit;
    searchEdit->setPlaceholderText(
        "🔎  Search free lectures by title, teacher, or category..."
        );
    searchEdit->setMinimumHeight(42);

    layout->addWidget(searchEdit);

    connect(
        searchEdit,
        &QLineEdit::textChanged,
        this,
        &MainWindow::searchBooks
        );

    layout->addSpacing(14);

    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    lectureHomeContainer = new QWidget;

    auto *grid = new QGridLayout(lectureHomeContainer);
    grid->setSpacing(18);
    grid->setContentsMargins(0, 0, 0, 0);

    scroll->setWidget(lectureHomeContainer);

    layout->addWidget(scroll, 1);

    return page;
}

QWidget *MainWindow::bookCard(const Book &book)
{
    auto *card = new QFrame;
    card->setObjectName("card");
    card->setMinimumHeight(210);

    auto *v = new QVBoxLayout(card);
    v->setContentsMargins(16, 14, 16, 14);

    auto *cover = new QLabel("📖");
    cover->setAlignment(Qt::AlignCenter);
    cover->setStyleSheet(QString("background:%1;color:%2;border-radius:9px;font-size:32px;").arg(INK, PARCHMENT));
    cover->setMinimumHeight(72);

    auto *title = new QLabel(book.title);
    title->setObjectName("bookTitle");
    title->setWordWrap(true);

    auto *author = new QLabel("by " + book.author);
    author->setObjectName("muted");

    auto *category = new QLabel(book.category);
    category->setObjectName("muted");

    auto *price = new QLabel(book.free ? "FREE" : QString("$%1").arg(book.price, 0, 'f', 2));
    price->setObjectName("tag");

    auto *open = new QPushButton(book.free ? "Open book" : "Buy book");
    open->setProperty("class", "primary");

    connect(open, &QPushButton::clicked, this, [this, book] {

        // Free books can be opened without an account.
        if (book.free) {
            showBookDialog(book);
            return;
        }

        // Paid books require the user to sign in first.
        if (currentUserName.isEmpty()) {
            QMessageBox::information(
                this,
                "Sign in required",
                "Please sign in with your Google account before buying this book."
                );

            showLogin();
            return;
        }

        // User is already signed in.
        showBookDialog(book);
    });

    v->addWidget(cover);
    v->addSpacing(8);
    v->addWidget(title);
    v->addWidget(author);
    v->addWidget(category);
    v->addWidget(price, 0, Qt::AlignLeft);
    v->addStretch();
    v->addWidget(open);
    return card;
}

void MainWindow::refreshHome()
{
    if (!lectureHomeContainer)
        return;

    auto *grid =
        static_cast<QGridLayout*>(lectureHomeContainer->layout());

    while (auto *item = grid->takeAt(0)) {
        if (item->widget())
            item->widget()->deleteLater();

        delete item;
    }

    const QString search =
        searchEdit
            ? searchEdit->text().trimmed().toLower()
            : QString();

    int row = 0;
    int col = 0;
    int count = 0;

    // HOME = FREE LECTURES ONLY
    for (const Lecture &lecture : lectures) {

        if (!lecture.free)
            continue;

        // Search title
        if (!search.isEmpty()) {

            const bool matchTitle =
                lecture.title.toLower().contains(search);

            const bool matchTeacher =
                lecture.teacher.toLower().contains(search);

            const bool matchCategory =
                lecture.category.toLower().contains(search);

            if (!matchTitle &&
                !matchTeacher &&
                !matchCategory) {

                continue;
            }
        }

        auto *card = lectureCard(lecture);

        grid->addWidget(card, row, col);

        ++col;
        ++count;

        if (col == 3) {
            col = 0;
            ++row;
        }
    }

    // No search results
    if (count == 0) {

        auto *empty = new QLabel(
            search.isEmpty()
                ? "No free lectures available."
                : "No lectures found. Try another search."
            );

        empty->setObjectName("muted");
        empty->setAlignment(Qt::AlignCenter);
        empty->setMinimumHeight(100);

        grid->addWidget(empty, 0, 0, 1, 3);
    }
}

void MainWindow::searchBooks() { refreshHome(); }
void MainWindow::filterBooks(int) { refreshHome(); }

QWidget *MainWindow::makeLecturesPage()
{
    auto *page = new QWidget;

    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);

    auto *title = new QLabel("Paid Lectures");
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    auto *sub = new QLabel(
        "Programming lectures and study materials — $0.03 per lecture."
        );
    sub->setObjectName("muted");
    layout->addWidget(sub);

    layout->addSpacing(12);

    // Search paid lectures
    paidLectureSearchEdit = new QLineEdit;
    paidLectureSearchEdit->setPlaceholderText(
        "Search paid lectures by title, teacher, or category..."
        );
    paidLectureSearchEdit->setMinimumHeight(42);

    layout->addWidget(paidLectureSearchEdit);

    connect(
        paidLectureSearchEdit,
        &QLineEdit::textChanged,
        this,
        &MainWindow::searchPaidLectures
        );

    layout->addSpacing(12);

    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    lectureContainer = new QWidget;

    auto *grid = new QGridLayout(lectureContainer);
    grid->setSpacing(16);

    scroll->setWidget(lectureContainer);

    layout->addWidget(scroll, 1);

    return page;
}

QWidget *MainWindow::lectureCard(const Lecture &lecture)
{
    auto *card = new QFrame;
    card->setObjectName("card");
    card->setMinimumHeight(220);

    auto *v = new QVBoxLayout(card);
    v->setContentsMargins(16, 14, 16, 14);

    const bool owned = !lecture.free && ownedLectures.contains(lecture.path);

    auto *icon = new QLabel;

    if (lecture.free) {
        icon->setText("📘  FREE");
        icon->setStyleSheet(
            QString("font-weight:700;color:%1;").arg(TEAL)
            );
    } else if (owned) {
        icon->setText("✅  PURCHASED");
        icon->setStyleSheet(
            QString("font-weight:700;color:%1;").arg(TEAL)
            );
    } else {
        icon->setText("🔒  PAID • $0.03");
        icon->setStyleSheet(
            "font-weight:700;color:#B87F1F;"
            );
    }

    auto *title = new QLabel(lecture.title);
    title->setObjectName("bookTitle");
    title->setWordWrap(true);

    auto *teacher = new QLabel(
        "Teacher: " + lecture.teacher
        );
    teacher->setObjectName("muted");

    auto *category = new QLabel(
        lecture.category
        );
    category->setObjectName("muted");

    // Rating summary shown directly on each lecture card.
    // Clicking it opens the full rating/comment dialog.
    auto *rating = new QLabel("<a href=\"rate\">★ —  |  0 ratings  |  0 likes</a>");
    rating->setTextFormat(Qt::RichText);
    rating->setTextInteractionFlags(Qt::TextBrowserInteraction);
    rating->setOpenExternalLinks(false);
    rating->setCursor(Qt::PointingHandCursor);
    connect(rating, &QLabel::linkActivated, this, [this, lecture](const QString &) {
        showLectureRatingDialog(lecture);
    });
    refreshLectureRating(lecture, rating);

    auto *button = new QPushButton;

    if (lecture.free) {
        button->setText("Open PDF");
        button->setProperty("class", "teal");
    } else if (owned) {
        button->setText("Read");
        button->setProperty("class", "teal");
    } else {
        button->setText("Buy • $0.03");
        button->setProperty("class", "primary");
    }

    connect(
        button,
        &QPushButton::clicked,
        this,
        [this, lecture] {
            showLectureDialog(lecture);
        }
        );

    v->addWidget(icon);
    v->addWidget(title);
    v->addWidget(teacher);
    v->addWidget(category);
    v->addWidget(rating);
    v->addStretch();
    v->addWidget(button);

    return card;
}

void MainWindow::refreshLectures()
{
    if (!lectureContainer)
        return;

    auto *grid =
        static_cast<QGridLayout*>(lectureContainer->layout());

    while (auto *item = grid->takeAt(0)) {
        if (item->widget())
            item->widget()->deleteLater();

        delete item;
    }

    const QString search =
        paidLectureSearchEdit
            ? paidLectureSearchEdit->text().trimmed().toLower()
            : QString();

    int row = 0;
    int col = 0;
    int count = 0;

    // PAID LECTURES ONLY
    for (const Lecture &lecture : lectures) {

        if (lecture.free)
            continue;

        // Search title, teacher, and category
        if (!search.isEmpty()) {
            const bool matchTitle =
                lecture.title.toLower().contains(search);

            const bool matchTeacher =
                lecture.teacher.toLower().contains(search);

            const bool matchCategory =
                lecture.category.toLower().contains(search);

            if (!matchTitle &&
                !matchTeacher &&
                !matchCategory) {
                continue;
            }
        }

        grid->addWidget(
            lectureCard(lecture),
            row,
            col
            );

        ++col;
        ++count;

        if (col == 3) {
            col = 0;
            ++row;
        }
    }

    // No search results
    if (count == 0) {
        auto *empty = new QLabel(
            search.isEmpty()
                ? "No paid lectures available."
                : "No paid lectures found. Try another search."
            );

        empty->setObjectName("muted");
        empty->setAlignment(Qt::AlignCenter);
        empty->setMinimumHeight(100);

        grid->addWidget(empty, 0, 0, 1, 3);
    }
}

void MainWindow::searchPaidLectures()
{
    refreshLectures();
}

void MainWindow::showBookDialog(const Book &book)
{
    // Paid books require authentication.
    if (!book.free && currentUserName.isEmpty()) {
        QMessageBox::information(
            this,
            "Sign in required",
            "Please sign in with your Google account before buying this book."
            );

        showLogin();
        return;
    }
    QDialog dlg(this);
    dlg.setWindowTitle(book.title);
    dlg.resize(760, 620);

    auto *v = new QVBoxLayout(&dlg);
    auto *title = new QLabel(book.title);
    title->setObjectName("pageTitle");
    v->addWidget(title);

    auto *meta = new QLabel(QString("By %1  •  %2  •  %3")
                                .arg(book.author, book.category, book.free ? "Free" : QString("$%1").arg(book.price, 0, 'f', 2)));
    meta->setObjectName("muted");
    v->addWidget(meta);
    v->addSpacing(8);

    auto *description = new QLabel(book.description);
    description->setWordWrap(true);
    v->addWidget(description);

    auto *reader = new QTextBrowser;
    reader->setPlainText(book.content);
    v->addWidget(reader, 1);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close);
    v->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    dlg.exec();
}

void MainWindow::refreshLectureRating(const Lecture &lecture, QLabel *ratingLabel)
{
    if (!ratingLabel || !networkManager) return;
    QUrl url(PAYMENTS_BACKEND_URL + "/api/ratings/" + QUrl::toPercentEncoding(lecture.path));
    QUrlQuery q; if (!currentUserEmail.isEmpty()) q.addQueryItem("email", currentUserEmail); url.setQuery(q);
    auto *reply = networkManager->get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [reply, ratingLabel] {
        const auto obj = QJsonDocument::fromJson(reply->readAll()).object(); reply->deleteLater();
        if (ratingLabel) ratingLabel->setText(QString("<a href=\"rate\">★ %1  |  %2 ratings  |  %3 likes</a>")
                                     .arg(obj.value("average").toDouble() > 0 ? QString::number(obj.value("average").toDouble(), 'f', 1) : "—")
                                     .arg(obj.value("count").toInt()).arg(obj.value("likes").toInt()));
    });
}

void MainWindow::showLectureRatingDialog(const Lecture &lecture)
{
    if (currentUserEmail.isEmpty()) { showLogin(); return; }
    QDialog dlg(this); dlg.setWindowTitle("Rate " + lecture.title); dlg.resize(620, 520);
    auto *v = new QVBoxLayout(&dlg);
    auto *title = new QLabel(lecture.title); title->setObjectName("pageTitle"); v->addWidget(title);
    auto *summary = new QLabel("Loading rating…"); v->addWidget(summary);
    auto *like = new QPushButton("♥ Like"); v->addWidget(like);
    auto *rating = new QComboBox; rating->addItems({"1", "2", "3", "4", "5"}); v->addWidget(rating);
    auto *comment = new QPlainTextEdit; comment->setPlaceholderText("Write a comment…"); comment->setMaximumHeight(120); v->addWidget(comment);
    auto *save = new QPushButton("Save rating & comment"); save->setProperty("class", "primary"); v->addWidget(save);
    auto *deleteMine = new QPushButton("Delete my rating & comment"); deleteMine->setEnabled(false); v->addWidget(deleteMine);
    auto *comments = new QTextBrowser; v->addWidget(comments, 1);

    // Any failed request (network error, backend down/asleep, bad
    // input, etc.) used to fail completely silently — the button just
    // looked like it did nothing. Surface it instead.
    auto reportIfFailed = [&dlg](QNetworkReply *r, const QJsonObject &o) -> bool {
        if (r->error() != QNetworkReply::NoError) {
            QMessageBox::warning(&dlg, "Couldn't reach server",
                "Request failed: " + r->errorString() +
                "\n\nIf the backend was idle it can take up to a minute to wake up — please try again.");
            return true;
        }
        if (o.contains("error")) {
            QMessageBox::warning(&dlg, "Request failed", o.value("error").toString());
            return true;
        }
        return false;
    };

    auto load = [this, &dlg, lecture, summary, like, rating, comment, comments, deleteMine]() {
        QUrl url(PAYMENTS_BACKEND_URL + "/api/ratings/" + QUrl::toPercentEncoding(lecture.path)); QUrlQuery q; q.addQueryItem("email", currentUserEmail); url.setQuery(q);
        auto *r = networkManager->get(QNetworkRequest(url));
        connect(r, &QNetworkReply::finished, &dlg, [this, r, &dlg, summary, like, rating, comment, comments, deleteMine] {
            if (r->error() != QNetworkReply::NoError) {
                summary->setText("Couldn't load rating (" + r->errorString() + ")");
                r->deleteLater();
                return;
            }
            auto o=QJsonDocument::fromJson(r->readAll()).object(); r->deleteLater();
            summary->setText(QString("Average: ★ %1 / 5   •   %2 ratings   •   %3 likes").arg(o.value("average").toDouble(),0,'f',1).arg(o.value("count").toInt()).arg(o.value("likes").toInt()));
            like->setText(o.value("liked").toBool() ? "♥ Liked" : "♡ Like");
            int my=o.value("myRating").toInt(); if(my>=1&&my<=5) rating->setCurrentText(QString::number(my));
            comment->setPlainText(o.value("myComment").toString());
            deleteMine->setEnabled(my>=1);
            const auto arr=o.value("comments").toArray(); QString html;
            for(const auto &x:arr){
                auto c=x.toObject();
                bool mine = c.value("mine").toBool();
                html += QString("<b>%1</b>%2  ★ %3<br>%4<hr>")
                            .arg(c.value("name").toString().toHtmlEscaped())
                            .arg(mine ? " <i>(you)</i>" : "")
                            .arg(c.value("rating").toInt())
                            .arg(c.value("comment").toString().toHtmlEscaped());
            }
            comments->setHtml(html.isEmpty()?"No comments yet.":html);
        });
    };
    connect(like, &QPushButton::clicked, &dlg, [this, lecture, load, like, reportIfFailed] {
        QNetworkRequest req(QUrl(PAYMENTS_BACKEND_URL+"/api/ratings/like")); req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
        QJsonObject b; b["itemId"]=lecture.path; b["userEmail"]=currentUserEmail; b["userName"]=currentUserName;
        auto *r=networkManager->post(req,QJsonDocument(b).toJson(QJsonDocument::Compact));
        connect(r,&QNetworkReply::finished,r,[r,load,reportIfFailed]{
            auto o=QJsonDocument::fromJson(r->readAll()).object();
            bool failed = reportIfFailed(r, o);
            r->deleteLater();
            if (!failed) load();
        });
    });
    connect(save, &QPushButton::clicked, &dlg, [this, lecture, rating, comment, load, reportIfFailed] {
        QNetworkRequest req(QUrl(PAYMENTS_BACKEND_URL+"/api/ratings")); req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
        QJsonObject b; b["itemId"]=lecture.path; b["userEmail"]=currentUserEmail; b["userName"]=currentUserName; b["rating"]=rating->currentText().toInt(); b["comment"]=comment->toPlainText();
        auto *r=networkManager->post(req,QJsonDocument(b).toJson(QJsonDocument::Compact));
        connect(r,&QNetworkReply::finished,r,[r,load,reportIfFailed]{
            auto o=QJsonDocument::fromJson(r->readAll()).object();
            bool failed = reportIfFailed(r, o);
            r->deleteLater();
            if (!failed) load();
        });
    });
    connect(deleteMine, &QPushButton::clicked, &dlg, [this, lecture, load, comment, &dlg, reportIfFailed] {
        if (QMessageBox::question(&dlg, "Delete comment", "Remove your rating and comment on this item?") != QMessageBox::Yes) return;
        QNetworkRequest req(QUrl(PAYMENTS_BACKEND_URL+"/api/ratings/delete")); req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
        QJsonObject b; b["itemId"]=lecture.path; b["userEmail"]=currentUserEmail;
        auto *r=networkManager->post(req,QJsonDocument(b).toJson(QJsonDocument::Compact));
        connect(r,&QNetworkReply::finished,r,[r,load,comment,reportIfFailed]{
            auto o=QJsonDocument::fromJson(r->readAll()).object();
            bool failed = reportIfFailed(r, o);
            r->deleteLater();
            if (!failed) { comment->clear(); load(); }
        });
    });
    load(); dlg.exec();
}

void MainWindow::showLectureDialog(const Lecture &lecture)
{
    if (!lecture.free) {

        if (currentUserName.isEmpty()) {
            QMessageBox::information(
                this,
                "Sign in required",
                "Please sign in with your Google account before buying this lecture."
                );

            showLogin();
            return;
        }

        // Already paid for this one under the signed-in account —
        // never charge twice, just open it.
        if (ownedLectures.contains(lecture.path)) {
            openPdf(lecture);
            return;
        }

        startLecturePayment(lecture);
        return;
    }

    openPdf(lecture);
}

void MainWindow::startLecturePayment(const Lecture &lecture)
{
    if (paymentDialog) {
        // A payment is already in progress; don't start another.
        paymentDialog->raise();
        paymentDialog->activateWindow();
        return;
    }

    if (currentUserEmail.isEmpty()) {
        showLogin();
        return;
    }

    // Already own it — belt and braces in case ownedLectures is stale;
    // the backend will also refuse to sell it again.
    if (ownedLectures.contains(lecture.path)) {
        openPdf(lecture);
        return;
    }

    paymentLecture = lecture;

    QNetworkRequest request(
        QUrl(PAYMENTS_BACKEND_URL + "/api/payments"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject body;
    body["lectureId"] = lecture.path;
    body["userEmail"] = currentUserEmail;

    QNetworkReply *reply = networkManager->post(
        request, QJsonDocument(body).toJson(QJsonDocument::Compact));

    // Build the QR dialog up front, styled like a bank KHQR payment
    // screen: clean KHQR header, a white card
    // with the KHQR mark, merchant/lecture name, amount, and QR.
    paymentDialog = new QDialog(this);
    paymentDialog->setWindowTitle("ABA KHQR");
    paymentDialog->setAttribute(Qt::WA_DeleteOnClose);
    paymentDialog->setFixedWidth(380);
    paymentDialog->setStyleSheet(
        "QDialog { background:#F3F1EC; }"
        );

    auto *outer = new QVBoxLayout(paymentDialog);
    outer->setContentsMargins(0, 0, 0, 20);
    outer->setSpacing(0);

    // ---- Header bar ----
    auto *header = new QFrame;
    header->setStyleSheet("background:#FFFFFF;");
    auto *hl = new QHBoxLayout(header);
    hl->setContentsMargins(16, 14, 16, 14);

    auto *cancel = new QPushButton("‹");
    cancel->setFixedSize(32, 32);
    cancel->setCursor(Qt::PointingHandCursor);
    cancel->setStyleSheet(
        "QPushButton { background:#F0F0F0; border:none; border-radius:8px; "
        "font-size:18px; color:#233046; }"
        "QPushButton:hover { background:#E4E4E4; }"
        );
    connect(cancel, &QPushButton::clicked, this, [this] {
        closePaymentDialog(false);
    });
    hl->addWidget(cancel);

    auto *khqrTitle = new QLabel("ABA KHQR");
    khqrTitle->setStyleSheet(
        "font-size:17px; font-weight:800; color:#233046; letter-spacing:0.5px;"
        );
    hl->addWidget(khqrTitle, 1, Qt::AlignCenter);

    // Keep the payment expiry timer internally, but do not display a
    // countdown to the customer while scanning the KHQR.
    paymentCountdownLabel = new QLabel(paymentDialog);
    paymentCountdownLabel->hide();

    outer->addWidget(header);
    outer->addSpacing(24);

    // ---- Ticket-style card ----
    auto *cardWrap = new QVBoxLayout;
    cardWrap->setContentsMargins(24, 0, 24, 0);

    auto *card = new QFrame;
    card->setStyleSheet(
        "QFrame { background:#FFFFFF; border-radius:16px; }"
        );
    auto *cv = new QVBoxLayout(card);
    cv->setContentsMargins(0, 0, 0, 18);
    cv->setSpacing(0);

    // Red header strip with the KHQR wordmark.
    auto *redStrip = new QFrame;
    redStrip->setFixedHeight(46);
    redStrip->setStyleSheet(
        "background:#E4192C; border-top-left-radius:16px; border-top-right-radius:16px;"
        );
    auto *rsl = new QHBoxLayout(redStrip);
    auto *khqrMark = new QLabel("KHQR");
    khqrMark->setStyleSheet(
        "color:white; font-size:16px; font-weight:800; letter-spacing:2px;"
        );
    rsl->addWidget(khqrMark, 0, Qt::AlignCenter);
    cv->addWidget(redStrip);
    cv->addSpacing(16);

    auto *merchant = new QLabel(lecture.title.toUpper());
    merchant->setAlignment(Qt::AlignCenter);
    merchant->setWordWrap(true);
    merchant->setStyleSheet(
        "color:#233046; font-size:13px; font-weight:700; letter-spacing:0.5px;"
        );
    cv->addWidget(merchant);
    cv->addSpacing(4);

    auto *amountRow = new QHBoxLayout;
    auto *amount = new QLabel("0.03");
    amount->setStyleSheet(
        "color:#233046; font-size:26px; font-weight:800;"
        );
    auto *currency = new QLabel(" USD");
    currency->setStyleSheet(
        "color:#7A8494; font-size:14px; font-weight:600; padding-top:8px;"
        );
    amountRow->addStretch();
    amountRow->addWidget(amount);
    amountRow->addWidget(currency);
    amountRow->addStretch();
    cv->addLayout(amountRow);
    cv->addSpacing(14);

    // Dashed separator, like the perforation on the real card.
    auto *dashed = new QFrame;
    dashed->setFixedHeight(1);
    dashed->setStyleSheet("border-top:2px dashed #E3DAC4; background:transparent;");
    cv->addWidget(dashed);
    cv->addSpacing(16);

    auto *qrImage = new QLabel;
    qrImage->setObjectName("qrImage");
    qrImage->setAlignment(Qt::AlignCenter);
    qrImage->setFixedSize(220, 220);
    qrImage->setStyleSheet("background:#FAFAFA; border-radius:8px;");
    qrImage->setText("Loading QR…");
    auto *qrRow = new QHBoxLayout;
    qrRow->addStretch();
    qrRow->addWidget(qrImage);
    qrRow->addStretch();
    cv->addLayout(qrRow);

    cardWrap->addWidget(card);
    outer->addLayout(cardWrap);
    outer->addSpacing(18);

    auto *footer = new QLabel(
        "Scan with mobile banking app\nthat supports KHQR");
    footer->setAlignment(Qt::AlignCenter);
    footer->setStyleSheet("color:#7A8494; font-size:13px;");
    outer->addWidget(footer);
    outer->addSpacing(10);

    paymentStatusLabel = new QLabel("Creating payment…");
    paymentStatusLabel->setAlignment(Qt::AlignCenter);
    paymentStatusLabel->setStyleSheet("color:#B87F1F; font-size:13px; font-weight:600;");
    outer->addWidget(paymentStatusLabel);

    connect(paymentDialog, &QDialog::destroyed, this, [this] {
        paymentDialog = nullptr;
        paymentStatusLabel = nullptr;
        paymentCountdownLabel = nullptr;
        if (paymentPollTimer) {
            paymentPollTimer->stop();
            paymentPollTimer->deleteLater();
            paymentPollTimer = nullptr;
        }
        if (paymentCountdownTimer) {
            paymentCountdownTimer->stop();
            paymentCountdownTimer->deleteLater();
            paymentCountdownTimer = nullptr;
        }
    });

    paymentDialog->show();

    connect(reply, &QNetworkReply::finished, this, [this, reply, qrImage] {
        const QByteArray json = reply->readAll();
        reply->deleteLater();

        if (!paymentDialog) return; // user already cancelled

        if (reply->error() != QNetworkReply::NoError) {
            paymentStatusLabel->setText(
                "Could not start payment: " + reply->errorString());
            return;
        }

        const QJsonObject obj = QJsonDocument::fromJson(json).object();

        // Backend already sold this to the signed-in account before —
        // no QR needed, just unlock it.
        if (obj.value("alreadyOwned").toBool()) {
            ownedLectures.insert(paymentLecture.path);
            paymentStatusLabel->setText("Already purchased — opening…");
            closePaymentDialog(true);
            return;
        }

        paymentId = obj.value("id").toString();
        const QString qr = obj.value("qr").toString();
        const QString qrLink = obj.value("qr_link").toString();
        const QString expiresAt = obj.value("expires_at").toString();

        if (paymentId.isEmpty()) {
            paymentStatusLabel->setText(
                "Backend did not return a payment id.\n" +
                QString::fromUtf8(json));
            return;
        }

        paymentStatusLabel->setText("Waiting for payment…");

        // Start the countdown. If the backend gave us an expiry
        // timestamp use it, otherwise assume the usual 3-minute window.
        const QDateTime parsedExpiry =
            QDateTime::fromString(expiresAt, Qt::ISODate);
        paymentExpiresAtMs = parsedExpiry.isValid()
                                 ? parsedExpiry.toMSecsSinceEpoch()
                                 : QDateTime::currentMSecsSinceEpoch() + 3 * 60 * 1000;

        updatePaymentCountdown();
        paymentCountdownTimer = new QTimer(this);
        connect(paymentCountdownTimer, &QTimer::timeout,
                this, &MainWindow::updatePaymentCountdown);
        paymentCountdownTimer->start(1000);

        // Tries to show the QR straight from the "qr" field in the
        // create-payment response, which most KHQR providers return as
        // a ready-to-display base64 PNG (optionally as a data: URI).
        // Returns true if that worked.
        auto tryShowEmbeddedQr = [qrImage](const QString &qrValue) -> bool {
            if (qrValue.isEmpty()) return false;

            QString base64Part = qrValue;
            const int commaIndex = base64Part.indexOf(',');
            if (base64Part.startsWith("data:") && commaIndex != -1)
                base64Part = base64Part.mid(commaIndex + 1);

            const QByteArray decoded = QByteArray::fromBase64(base64Part.toUtf8());
            QPixmap pix;
            if (!decoded.isEmpty() && pix.loadFromData(decoded)) {
                qrImage->setPixmap(
                    pix.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                return true;
            }
            return false;
        };

        // Falls back to fetching the QR as an image from qr_link over
        // the network — only used when the response didn't already
        // include usable embedded QR data.
        auto fetchQrFromLink = [this, qrImage](const QString &link) {
            if (link.isEmpty()) {
                qrImage->setText("No QR available for this payment.");
                return;
            }

            QNetworkReply *imgReply = networkManager->get(QNetworkRequest(QUrl(link)));
            connect(imgReply, &QNetworkReply::finished, this, [imgReply, qrImage] {
                imgReply->deleteLater();
                if (imgReply->error() != QNetworkReply::NoError) {
                    qrImage->setText("Could not load QR image.");
                    return;
                }
                QPixmap pix;
                if (pix.loadFromData(imgReply->readAll())) {
                    qrImage->setPixmap(
                        pix.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                } else {
                    qrImage->setText("Could not decode QR image.");
                }
            });
        };

        if (!tryShowEmbeddedQr(qr))
            fetchQrFromLink(qrLink);


        // Poll our backend for the payment status every 3 seconds.
        paymentPollTimer = new QTimer(this);
        connect(paymentPollTimer, &QTimer::timeout, this, &MainWindow::pollPaymentStatus);
        paymentPollTimer->start(3000);
    });
}

void MainWindow::updatePaymentCountdown()
{
    if (!paymentCountdownLabel)
        return;

    const qint64 remainingMs =
        paymentExpiresAtMs - QDateTime::currentMSecsSinceEpoch();

    if (remainingMs <= 0) {
        paymentCountdownLabel->setText("00:00");
        paymentCountdownLabel->setStyleSheet(
            "font-size:14px; font-weight:700; color:#E4192C;");
        if (paymentStatusLabel)
            paymentStatusLabel->setText("QR code expired — please try again.");
        if (paymentCountdownTimer) paymentCountdownTimer->stop();
        if (paymentPollTimer) paymentPollTimer->stop();
        return;
    }

    const int totalSeconds = static_cast<int>(remainingMs / 1000);
    const int mm = totalSeconds / 60;
    const int ss = totalSeconds % 60;
    paymentCountdownLabel->setText(
        QString("%1:%2")
            .arg(mm, 2, 10, QChar('0'))
            .arg(ss, 2, 10, QChar('0')));

    if (remainingMs < 30000) {
        paymentCountdownLabel->setStyleSheet(
            "font-size:14px; font-weight:700; color:#E4192C;");
    }
}

void MainWindow::pollPaymentStatus()
{
    if (paymentId.isEmpty() || !paymentDialog)
        return;

    QNetworkRequest request(QUrl(
        PAYMENTS_BACKEND_URL + "/api/payments/" + paymentId + "/status"));

    QNetworkReply *reply = networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        const QByteArray json = reply->readAll();
        reply->deleteLater();

        if (!paymentDialog) return;

        if (reply->error() != QNetworkReply::NoError) {
            // Transient network hiccup; just try again next tick.
            return;
        }

        const QJsonObject obj = QJsonDocument::fromJson(json).object();
        const QString status = obj.value("status").toString();

        if (status == "paid") {
            paymentStatusLabel->setText("Payment received!");
            closePaymentDialog(true);
        } else if (status == "failed" || status == "expired") {
            paymentStatusLabel->setText(
                status == "expired" ? "Payment expired." : "Payment failed.");
            if (paymentPollTimer) paymentPollTimer->stop();
            if (paymentCountdownTimer) paymentCountdownTimer->stop();
        } else if (!status.isEmpty()) {
            paymentStatusLabel->setText("Status: " + status + "…");
        }
    });
}

void MainWindow::closePaymentDialog(bool unlocked)
{
    const Lecture lecture = paymentLecture;

    if (paymentPollTimer) {
        paymentPollTimer->stop();
    }
    if (paymentCountdownTimer) {
        paymentCountdownTimer->stop();
    }

    if (paymentDialog) {
        paymentDialog->close(); // triggers WA_DeleteOnClose cleanup
    }

    paymentId.clear();

    if (unlocked) {
        // Remember it locally so the lecture list / My Books flip to
        // "Read" immediately, without waiting on another network call.
        ownedLectures.insert(lecture.path);
        refreshLectures();
        refreshMyBooks();
        openPdf(lecture);
    }
}

void MainWindow::openPdf(const Lecture &lecture)
{
    const QString path = assetPath(lecture.path);
    if (!QFileInfo::exists(path)) {
        QMessageBox::warning(this, "PDF not found",
                             "The PDF file was not found beside the application:\n\n" + path +
                                 "\n\nCopy the provided assets folder beside the .exe.");
        return;
    }
    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(path))) {
        QMessageBox::warning(this, "Cannot open PDF", path);
    }
}

void MainWindow::showHome()
{
    if (pages) pages->setCurrentIndex(0);
    if (navigation && navigation->currentRow() != 0) navigation->setCurrentRow(0);
    refreshHome();
}

void MainWindow::showLectures()
{
    if (pages) pages->setCurrentIndex(1);
    if (navigation && navigation->currentRow() != 1) navigation->setCurrentRow(1);
    refreshLectures();
}

void MainWindow::showMyBooks()
{
    if (pages) pages->setCurrentIndex(2);
    if (navigation && navigation->currentRow() != 2) navigation->setCurrentRow(2);
    refreshMyBooks();
}

void MainWindow::showAdmin()
{
    if (currentUserName.isEmpty()) {
        showLogin();
        return;
    }
    if (!admin) {
        QMessageBox::information(this, "Admin", "Admin area is available after signing in as the demo administrator.");
        return;
    }
    if (pages) pages->setCurrentIndex(3);
    if (navigation && navigation->currentRow() != 3) navigation->setCurrentRow(3);
    refreshAdminStats();
}

QWidget *MainWindow::makeMyBooksPage()
{
    auto *page = new QWidget;
    auto *v = new QVBoxLayout(page);
    v->setContentsMargins(28, 28, 28, 28);

    auto *title = new QLabel("My Books");
    title->setObjectName("pageTitle");
    v->addWidget(title);

    auto *info = new QLabel(
        "Lectures you've already paid for. They're tied to your signed-in "
        "account, so you'll never be charged for the same one twice — open "
        "them any time.");
    info->setObjectName("muted");
    info->setWordWrap(true);
    v->addWidget(info);
    v->addSpacing(12);

    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    myBooksContainer = new QWidget;
    auto *grid = new QGridLayout(myBooksContainer);
    grid->setSpacing(16);

    scroll->setWidget(myBooksContainer);
    v->addWidget(scroll, 1);

    return page;
}

void MainWindow::refreshMyBooks()
{
    if (!myBooksContainer)
        return;

    auto *grid = static_cast<QGridLayout*>(myBooksContainer->layout());
    while (auto *item = grid->takeAt(0)) {
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }

    if (currentUserEmail.isEmpty()) {
        auto *msg = new QLabel(
            "Sign in to see the lectures you've purchased.");
        msg->setObjectName("muted");
        grid->addWidget(msg, 0, 0);
        return;
    }

    QList<Lecture> owned;
    for (const Lecture &lecture : lectures) {
        if (!lecture.free && ownedLectures.contains(lecture.path))
            owned.append(lecture);
    }

    if (owned.isEmpty()) {
        auto *msg = new QLabel(
            "You haven't bought any lectures yet. "
            "Purchased lectures will show up here and stay unlocked forever.");
        msg->setObjectName("muted");
        msg->setWordWrap(true);
        grid->addWidget(msg, 0, 0);
        return;
    }

    int row = 0, col = 0;
    for (const Lecture &lecture : owned) {
        grid->addWidget(lectureCard(lecture), row, col);
        if (++col >= 3) { col = 0; ++row; }
    }
}

void MainWindow::fetchOwnedLectures()
{
    if (currentUserEmail.isEmpty() || !networkManager) {
        ownedLectures.clear();
        refreshLectures();
        refreshMyBooks();
        return;
    }

    QNetworkRequest request(QUrl(
        PAYMENTS_BACKEND_URL + "/api/purchases/" + currentUserEmail));

    QNetworkReply *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError)
            return; // stay with whatever we had; try again next time

        const QJsonObject obj =
            QJsonDocument::fromJson(reply->readAll()).object();
        const QJsonArray ids = obj.value("lectureIds").toArray();

        ownedLectures.clear();
        for (const QJsonValue &id : ids)
            ownedLectures.insert(id.toString());

        refreshLectures();
        refreshMyBooks();
    });
}

// Tells the backend a sign-in just happened (for the admin dashboard's
// login counts / "last seen"), and starts the periodic heartbeat that
// keeps this user marked "online" while the app stays open.
void MainWindow::notifyLogin()
{
    if (currentUserEmail.isEmpty() || !networkManager)
        return;

    QNetworkRequest request(QUrl(PAYMENTS_BACKEND_URL + "/api/login"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject body;
    body["email"] = currentUserEmail;
    body["name"] = currentUserName;

    QNetworkReply *reply = networkManager->post(
        request, QJsonDocument(body).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);

    if (heartbeatTimer) heartbeatTimer->start();
    sendHeartbeat();
}

// Periodic "I'm still here" ping so the admin dashboard's online/offline
// status stays accurate. Fire-and-forget — a missed ping just means the
// user shows as offline a little sooner, which is fine.
void MainWindow::sendHeartbeat()
{
    if (currentUserEmail.isEmpty() || !networkManager)
        return;

    QNetworkRequest request(QUrl(PAYMENTS_BACKEND_URL + "/api/heartbeat"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject body;
    body["email"] = currentUserEmail;

    QNetworkReply *reply = networkManager->post(
        request, QJsonDocument(body).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
}

// Pulls per-user stats (logins, online status, books bought/owned) from
// the backend and fills in the admin page's summary cards and table.
void MainWindow::refreshAdminStats()
{
    if (!admin || currentUserEmail.isEmpty() || !networkManager || !adminUsersTable)
        return;

    QUrl url(PAYMENTS_BACKEND_URL + "/api/admin/users");
    QUrlQuery q;
    q.addQueryItem("adminEmail", currentUserEmail);
    url.setQuery(q);

    QNetworkReply *reply = networkManager->get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError)
            return;

        const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
        const QJsonArray users = obj.value("users").toArray();

        int onlineCount = 0;
        int totalBought = 0;

        adminUsersTable->setRowCount(users.size());
        for (int i = 0; i < users.size(); ++i) {
            const QJsonObject u = users[i].toObject();
            const bool online = u.value("online").toBool();
            const int loginCount = u.value("loginCount").toInt();
            const int bought = u.value("booksBought").toInt();
            const int owned = u.value("booksOwned").toInt();
            const double spent = u.value("totalSpent").toDouble();

            if (online) ++onlineCount;
            totalBought += bought;

            adminUsersTable->setItem(i, 0, new QTableWidgetItem(u.value("name").toString()));
            adminUsersTable->setItem(i, 1, new QTableWidgetItem(u.value("email").toString()));
            auto *status = new QTableWidgetItem(online ? "🟢 Online" : "⚪ Offline");
            status->setForeground(online ? QColor(TEAL) : QColor(INK_SOFT));
            adminUsersTable->setItem(i, 2, status);
            adminUsersTable->setItem(i, 3, new QTableWidgetItem(QString::number(loginCount)));
            adminUsersTable->setItem(i, 4, new QTableWidgetItem(QString::number(bought)));
            adminUsersTable->setItem(i, 5, new QTableWidgetItem(QString::number(owned)));
            adminUsersTable->setItem(i, 6, new QTableWidgetItem(QString("$%1").arg(spent, 0, 'f', 2)));
        }

        if (adminUsersCountValue) adminUsersCountValue->setText(QString::number(users.size()));
        if (adminOnlineCountValue) adminOnlineCountValue->setText(QString::number(onlineCount));
        if (adminBooksBoughtValue) adminBooksBoughtValue->setText(QString::number(totalBought));
        if (adminRevenueValue) adminRevenueValue->setText(QString("$%1").arg(obj.value("totalRevenue").toDouble(), 0, 'f', 2));
    });
}

void MainWindow::addAdminItem()
{
    if (!admin) return;
    QDialog dlg(this); dlg.setWindowTitle("Add Book / Lecture"); dlg.resize(520, 480);
    auto *form=new QFormLayout(&dlg); auto *type=new QComboBox; type->addItems({"Book","Lecture"});
    auto *name=new QLineEdit; auto *author=new QLineEdit; auto *category=new QLineEdit("C++ / Programming"); auto *price=new QLineEdit("0.00");
    auto *path=new QLineEdit; path->setPlaceholderText("For lecture: paid/your-file.pdf"); auto *content=new QPlainTextEdit; content->setPlaceholderText("Book content (for a book)");
    form->addRow("Type",type); form->addRow("Name",name); form->addRow("Author / Teacher",author); form->addRow("Category",category); form->addRow("Price (USD)",price); form->addRow("PDF path",path); form->addRow("Book content",content);
    auto *save=new QPushButton("Add"); form->addRow(save);
    connect(save,&QPushButton::clicked,&dlg,[this,&dlg,type,name,author,category,price,path,content]{
        if(name->text().trimmed().isEmpty()){QMessageBox::warning(&dlg,"Missing name","Enter a name.");return;}
        bool ok=false; double p=price->text().toDouble(&ok); if(!ok||p<0){QMessageBox::warning(&dlg,"Invalid price","Enter a valid USD price.");return;}
        const QString file=QCoreApplication::applicationDirPath()+"/admin_catalog.json"; QJsonArray arr; QFile f(file); if(f.open(QIODevice::ReadOnly)){auto d=QJsonDocument::fromJson(f.readAll());arr=d.array();f.close();}
        QJsonObject o; o["id"]=QUuid::createUuid().toString(QUuid::WithoutBraces); o["type"]=type->currentText().toLower(); o["title"]=name->text().trimmed(); o["author"]=author->text().trimmed(); o["category"]=category->text().trimmed(); o["price"]=p; o["isFree"]=p<=0; o["path"]=path->text().trimmed(); o["content"]=content->toPlainText(); arr.append(o);
        if(!f.open(QIODevice::WriteOnly)){QMessageBox::warning(&dlg,"Save failed","Cannot write admin_catalog.json beside the app.");return;} f.write(QJsonDocument(arr).toJson(QJsonDocument::Indented)); f.close();
        QMessageBox::information(&dlg,"Added","The item was saved. Restart the app to load the new content."); dlg.accept();
    }); dlg.exec();
}

QWidget *MainWindow::makeAdminPage()
{
    auto *page = new QWidget;
    auto *v = new QVBoxLayout(page);
    v->setContentsMargins(28, 28, 28, 28);

    auto *title = new QLabel("Admin Dashboard");
    title->setObjectName("pageTitle");
    v->addWidget(title);

    auto *stats = new QHBoxLayout;
    const QStringList labels = {"Lectures", "Signed-up Users", "Online Now", "Lectures Bought", "Total Revenue"};
    QList<QLabel**> targets = {nullptr, &adminUsersCountValue, &adminOnlineCountValue, &adminBooksBoughtValue, &adminRevenueValue};
    const QStringList values = {
        QString::number(lectures.size()),
        "—",
        "—",
        "—",
        "—",
    };
    for (int i = 0; i < labels.size(); ++i) {
        auto *box = new QFrame;
        box->setObjectName("card");
        auto *bv = new QVBoxLayout(box);
        auto *num = new QLabel(values[i]);
        num->setStyleSheet("font-size:28px;font-weight:700;");
        auto *lab = new QLabel(labels[i]);
        lab->setObjectName("muted");
        bv->addWidget(num);
        bv->addWidget(lab);
        stats->addWidget(box);
        if (targets[i]) *targets[i] = num;
    }
    v->addLayout(stats);
    v->addSpacing(14);

    auto *usersTitle = new QLabel("User Activity");
    usersTitle->setStyleSheet("font-weight:700;");
    v->addWidget(usersTitle);

    auto *usersInfo = new QLabel(
        "Logins, online status, and lecture purchases per signed-in user. "
        "\"Online\" means we heard from that user's app in the last 90 seconds.");
    usersInfo->setObjectName("muted");
    usersInfo->setWordWrap(true);
    v->addWidget(usersInfo);

    adminUsersTable = new QTableWidget(0, 7);
    adminUsersTable->setHorizontalHeaderLabels(
        {"Name", "Email", "Status", "Logins", "Books Bought", "Books Owned", "Spent"});
    adminUsersTable->horizontalHeader()->setStretchLastSection(true);
    adminUsersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    v->addWidget(adminUsersTable, 1);

    return page;
}

QWidget *MainWindow::makeLoginPage()
{
    auto *page = new QWidget;
    auto *outer = new QVBoxLayout(page);
    outer->setContentsMargins(80, 60, 80, 60);

    auto *box = new QFrame;
    box->setObjectName("card");
    box->setMaximumWidth(520);
    auto *v = new QVBoxLayout(box);
    v->setContentsMargins(28, 28, 28, 28);

    auto *title = new QLabel("Welcome to Sabai Books");
    title->setObjectName("pageTitle");
    v->addWidget(title);

    auto *sub = new QLabel("Sign in with your Google account to continue.");
    sub->setObjectName("muted");
    sub->setWordWrap(true);
    v->addWidget(sub);
    v->addSpacing(16);

    auto *google = new QPushButton("Continue with Google");
    google->setProperty("class", "primary");
    google->setMinimumHeight(44);
    google->setCursor(Qt::PointingHandCursor);
    v->addWidget(google);

    auto *status = new QLabel("Your browser will open for the secure Google sign-in.");
    status->setObjectName("muted");
    status->setWordWrap(true);
    v->addWidget(status);

    connect(google, &QPushButton::clicked, this, [this, status] {
        status->setText("Opening Google sign-in in your browser...");
        startGoogleLogin();
    });

    outer->addWidget(box, 0, Qt::AlignHCenter);
    outer->addStretch();
    return page;
}

void MainWindow::startGoogleLogin()
{
    if (GOOGLE_CLIENT_ID.startsWith("PASTE_")) {
        QMessageBox::information(this, "Google Login Setup",
                                 "Google Login is ready, but the Google Desktop OAuth Client ID is not configured yet.\n\n"
                                 "Create a Desktop OAuth client in Google Cloud Console, then paste its Client ID into mainwindow.cpp.");
        return;
    }
    if (!oauthServer->listen(QHostAddress::LocalHost, 0)) {
        QMessageBox::warning(this, "Google Login", "Could not start the local OAuth callback server."); return;
    }
    oauthRedirectUri = QString("http://127.0.0.1:%1").arg(oauthServer->serverPort());
    QByteArray randomBytes(48, Qt::Uninitialized);
    for (int i=0;i<randomBytes.size();++i) randomBytes[i]=static_cast<char>(QRandomGenerator::global()->generate() & 0xFF);
    oauthCodeVerifier=QString::fromLatin1(randomBytes.toBase64(QByteArray::Base64UrlEncoding|QByteArray::OmitTrailingEquals));
    const QByteArray challenge=QCryptographicHash::hash(oauthCodeVerifier.toLatin1(),QCryptographicHash::Sha256).toBase64(QByteArray::Base64UrlEncoding|QByteArray::OmitTrailingEquals);
    QByteArray stateBytes(24, Qt::Uninitialized);
    for (int i=0;i<stateBytes.size();++i) stateBytes[i]=static_cast<char>(QRandomGenerator::global()->generate() & 0xFF);
    oauthState=QString::fromLatin1(stateBytes.toBase64(QByteArray::Base64UrlEncoding|QByteArray::OmitTrailingEquals));
    QUrl url("https://accounts.google.com/o/oauth2/v2/auth"); QUrlQuery q;
    q.addQueryItem("client_id",GOOGLE_CLIENT_ID); q.addQueryItem("redirect_uri",oauthRedirectUri);
    q.addQueryItem("response_type","code"); q.addQueryItem("scope","openid email profile");
    q.addQueryItem("state",oauthState); q.addQueryItem("code_challenge",QString::fromLatin1(challenge)); q.addQueryItem("code_challenge_method","S256");
    url.setQuery(q);
    if (!QDesktopServices::openUrl(url)) { oauthServer->close(); QMessageBox::warning(this,"Google Login","Could not open your browser."); }
}

void MainWindow::handleGoogleConnection()
{
    while (oauthServer->hasPendingConnections()) {
        QTcpSocket *socket=oauthServer->nextPendingConnection();
        connect(socket,&QTcpSocket::readyRead,this,[this,socket]{
            const QByteArray request=socket->readAll(); const QList<QByteArray> lines=request.split('\n'); if(lines.isEmpty()) return;
            const QList<QByteArray> parts=lines.first().trimmed().split(' '); if(parts.size()<2) return;
            QUrl callback(QString("http://127.0.0.1")+QString::fromUtf8(parts[1])); QUrlQuery query(callback);
            const QString code=query.queryItemValue("code"), state=query.queryItemValue("state"), error=query.queryItemValue("error");
            const QByteArray response =
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html; charset=utf-8\r\n"
                "Connection: close\r\n"
                "\r\n"
                "<!DOCTYPE html>"
                "<html>"
                "<head>"
                "<meta charset='UTF-8'>"
                "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
                "<title>Sabai Books - Sign In</title>"
                "<style>"
                "* { box-sizing: border-box; }"
                "body {"
                "margin: 0;"
                "min-height: 100vh;"
                "display: flex;"
                "align-items: center;"
                "justify-content: center;"
                "font-family: 'Segoe UI', Arial, sans-serif;"
                "background: #F7F3E9;"
                "color: #1B2430;"
                "}"
                ".card {"
                "width: 440px;"
                "background: #FFFFFF;"
                "border: 1px solid #E3DAC4;"
                "border-radius: 16px;"
                "padding: 42px 38px;"
                "text-align: center;"
                "box-shadow: 0 12px 35px rgba(27,36,48,.12);"
                "}"
                ".logo {"
                "width: 64px;"
                "height: 64px;"
                "margin: 0 auto 20px;"
                "border-radius: 14px;"
                "background: #1B2430;"
                "display: flex;"
                "align-items: center;"
                "justify-content: center;"
                "font-size: 30px;"
                "}"
                "h1 {"
                "margin: 0 0 10px;"
                "font-size: 25px;"
                "font-weight: 700;"
                "}"
                ".subtitle {"
                "margin: 0 0 28px;"
                "font-size: 14px;"
                "color: #667085;"
                "line-height: 1.6;"
                "}"
                ".success {"
                "display: flex;"
                "align-items: center;"
                "gap: 12px;"
                "padding: 14px 16px;"
                "margin-bottom: 24px;"
                "background: #EEF8F4;"
                "border: 1px solid #CBE7DD;"
                "border-radius: 10px;"
                "text-align: left;"
                "color: #246B5D;"
                "font-size: 14px;"
                "font-weight: 600;"
                "}"
                ".check {"
                "width: 28px;"
                "height: 28px;"
                "border-radius: 50%;"
                "background: #2F7A6F;"
                "color: white;"
                "display: flex;"
                "align-items: center;"
                "justify-content: center;"
                "font-size: 16px;"
                "flex-shrink: 0;"
                "}"
                ".button {"
                "display: inline-block;"
                "padding: 11px 22px;"
                "border-radius: 8px;"
                "background: #E4A335;"
                "color: #1B2430;"
                "font-size: 14px;"
                "font-weight: 700;"
                "text-decoration: none;"
                "}"
                ".footer {"
                "margin-top: 28px;"
                "padding-top: 18px;"
                "border-top: 1px solid #E9E4D8;"
                "font-size: 12px;"
                "color: #8A93A1;"
                "}"
                "</style>"
                "</head>"
                "<body>"
                "<div class='card'>"
                "<div class='logo'>📖</div>"
                "<h1>Sabai Books</h1>"
                "<p class='subtitle'>Google authentication completed successfully.</p>"
                "<div class='success'>"
                "<div class='check'>✓</div>"
                "<div>Your Google account has been verified.</div>"
                "</div>"
                "<a class='button' href='javascript:window.close();'>"
                "Close this window"
                "</a>"
                "<div class='footer'>"
                "You can now return to the Sabai Books application."
                "</div>"
                "</div>"
                "</body>"
                "</html>";
            socket->write(response); socket->disconnectFromHost(); oauthServer->close();
            if(!error.isEmpty()){QMessageBox::information(this,"Google Login","Google sign-in was cancelled or denied.");return;}
            if(state!=oauthState||code.isEmpty()){QMessageBox::warning(this,"Google Login","The Google login response could not be verified.");return;}
            exchangeGoogleCode(code);
        });
    }
}


void MainWindow::exchangeGoogleCode(const QString &code)
{
    QNetworkRequest request(
        QUrl("https://oauth2.googleapis.com/token")
        );

    request.setHeader(
        QNetworkRequest::ContentTypeHeader,
        "application/x-www-form-urlencoded"
        );

    QUrlQuery body;
    body.addQueryItem("client_id", GOOGLE_CLIENT_ID);
    body.addQueryItem("client_secret", GOOGLE_CLIENT_SECRET);
    body.addQueryItem("code", code);
    body.addQueryItem("code_verifier", oauthCodeVerifier);
    body.addQueryItem("grant_type", "authorization_code");
    body.addQueryItem("redirect_uri", oauthRedirectUri);

    QNetworkReply *reply =
        networkManager->post(
            request,
            body.toString(QUrl::FullyEncoded).toUtf8()
            );

    connect(reply, &QNetworkReply::finished,
            this, [this, reply] {

                const QByteArray json = reply->readAll();

                const int status =
                    reply->attribute(
                             QNetworkRequest::HttpStatusCodeAttribute
                             ).toInt();

                if (reply->error() != QNetworkReply::NoError) {
                    QMessageBox::warning(
                        this,
                        "Google Login",
                        "Network error:\n\n" +
                            reply->errorString() +
                            "\n\nResponse:\n" +
                            QString::fromUtf8(json)
                        );

                    reply->deleteLater();
                    return;
                }

                if (status < 200 || status >= 300) {
                    QMessageBox::warning(
                        this,
                        "Google Login",
                        "Google token exchange failed.\n\n"
                        "HTTP status: " +
                            QString::number(status) +
                            "\n\nResponse:\n" +
                            QString::fromUtf8(json)
                        );

                    reply->deleteLater();
                    return;
                }

                const QJsonObject obj =
                    QJsonDocument::fromJson(json).object();

                const QString access =
                    obj.value("access_token").toString();

                if (access.isEmpty()) {
                    QMessageBox::warning(
                        this,
                        "Google Login",
                        "Google did not return an access token.\n\n" +
                            QString::fromUtf8(json)
                        );

                    reply->deleteLater();
                    return;
                }

                reply->deleteLater();

                QNetworkRequest userReq(
                    QUrl(
                        "https://openidconnect.googleapis.com/v1/userinfo"
                        )
                    );

                userReq.setRawHeader(
                    "Authorization",
                    "Bearer " + access.toUtf8()
                    );

                QNetworkReply *userReply =
                    networkManager->get(userReq);

                connect(
                    userReply,
                    &QNetworkReply::finished,
                    this,
                    [this, userReply] {

                        const QByteArray profile =
                            userReply->readAll();

                        const int status =
                            userReply->attribute(
                                         QNetworkRequest::HttpStatusCodeAttribute
                                         ).toInt();

                        if (userReply->error() !=
                            QNetworkReply::NoError) {

                            QMessageBox::warning(
                                this,
                                "Google Login",
                                "Could not read Google profile:\n\n" +
                                    userReply->errorString()
                                );

                            userReply->deleteLater();
                            return;
                        }

                        if (status < 200 || status >= 300) {

                            QMessageBox::warning(
                                this,
                                "Google Login",
                                "Google profile request failed."
                                );

                            userReply->deleteLater();
                            return;
                        }

                        userReply->deleteLater();

                        finishGoogleLogin(profile);
                    }
                    );
            });
}

void MainWindow::finishGoogleLogin(const QByteArray &json)
{
    const QJsonObject o=QJsonDocument::fromJson(json).object(); const QString email=o.value("email").toString().trimmed().toLower(); const QString name=o.value("name").toString().trimmed();
    if(email.isEmpty()){QMessageBox::warning(this,"Google Login","Google did not return an email address.");return;}
    currentUserEmail=email; currentUserName=name.isEmpty()?email:name; admin=currentUserEmail==ADMIN_EMAIL; updateUserLabel(); notifyLogin(); fetchOwnedLectures(); showHome();
    QMessageBox::information(this,"Welcome","Signed in with Google as:\n"+currentUserEmail);
}

void MainWindow::showLogin()
{
    pages->setCurrentIndex(4);
}

void MainWindow::logout()
{
    currentUserName.clear();
    currentUserEmail.clear();
    admin = false;
    ownedLectures.clear();
    if (heartbeatTimer) heartbeatTimer->stop();
    updateUserLabel();
    refreshLectures();
    refreshMyBooks();
    showHome();
}

void MainWindow::updateUserLabel()
{
    if (!userLabel)
        return;

    if (currentUserName.isEmpty()) {
        userLabel->setText("Not signed in");
    } else {
        userLabel->setText(
            currentUserName + (admin ? "  •  Admin" : "")
            );
    }

    auto *header = userLabel->parentWidget();

    if (header) {
        auto *button = header->findChild<QPushButton*>("headerButton");

        if (button) {
            button->setText(
                currentUserName.isEmpty() ? "Log in" : "Log out"
                );
        }
    }

    // Only the admin account should even see the Admin item in the
    // sidebar — everyone else shouldn't know it's there.
    if (navigation)
        navigation->setRowHidden(3, !admin);
}

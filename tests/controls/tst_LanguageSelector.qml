import QtQuick
import QtTest
import App.Controls

TestCase {
    id: testCase
    name: "LanguageSelector"
    width: 300
    height: 250
    visible: true
    when: windowShown
    property string selectedLanguage: ""

    function init(): void {
        failOnWarning(/.?/);
        testCase.selectedLanguage = "";
    }

    Component {
        id: selectorComponent
        LanguageSelector {
            language: "en"
            hoverEnabled: true
            onLanguageSelected: language => testCase.selectedLanguage = language
        }
    }

    function test_clickSelectsGerman(): void {
        const selector = createTemporaryObject(selectorComponent, testCase);
        mouseClick(selector);
        const menu = findChild(selector, "languageMenu");
        tryVerify(() => menu.visible);
        const german = findChild(menu.contentItem, "language_de");
        verify(german);
        mouseClick(german);
        compare(testCase.selectedLanguage, "de");
        tryVerify(() => !menu.visible);
    }

    function test_keyboardSelectsSpanish(): void {
        const selector = createTemporaryObject(selectorComponent, testCase);
        selector.forceActiveFocus();
        keyClick(Qt.Key_Space);
        const menu = findChild(selector, "languageMenu");
        tryVerify(() => menu.visible);
        keyClick(Qt.Key_Down);
        keyClick(Qt.Key_Down);
        keyClick(Qt.Key_Return);
        compare(testCase.selectedLanguage, "es");
    }

    function test_hoverOpensMenu(): void {
        const selector = createTemporaryObject(selectorComponent, testCase);
        mouseMove(selector);
        const menu = findChild(selector, "languageMenu");
        tryVerify(() => menu.visible);
    }

    function test_languageCodeBadgeIsDistinct(): void {
        const selector = createTemporaryObject(selectorComponent, testCase);
        mouseClick(selector);
        const menu = findChild(selector, "languageMenu");
        tryVerify(() => menu.visible);
        const german = findChild(menu.contentItem, "language_de");
        const badge = findChild(german, "languageCodeBadge");
        verify(badge);
        compare(badge.border.width, 1);
        verify(Qt.colorEqual(badge.color, Theme.hover));
        verify(Qt.colorEqual(badge.border.color, Theme.border));
    }

    function test_keyboardSelectsGerman(): void {
        const selector = createTemporaryObject(selectorComponent, testCase);
        selector.forceActiveFocus();
        keyClick(Qt.Key_Space);
        const menu = findChild(selector, "languageMenu");
        tryVerify(() => menu.visible);
        keyClick(Qt.Key_Down);
        keyClick(Qt.Key_Return);
        compare(testCase.selectedLanguage, "de");
        tryVerify(() => !menu.visible);
    }
}

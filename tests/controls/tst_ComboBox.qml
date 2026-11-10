import QtQuick
import QtTest
import App.Controls

TestCase {
    id: testCase
    name: "ComboBox"
    width: 300
    height: 300
    visible: true
    when: windowShown

    function init(): void {
        failOnWarning(/.?/);
    }

    Component {
        id: comboComponent
        ComboBox {
            width: 180
            model: ["Plain", "Dots", "Ruled"]
        }
    }

    function test_borderAndSelection(): void {
        ignoreWarning(/Populating font family aliases took .*Replace uses of missing font family "Sans Serif".*/);
        const combo = createTemporaryObject(comboComponent, testCase);
        compare(combo.background.border.width, 1);
        mouseClick(combo);
        tryVerify(() => combo.popup.visible);
        keyClick(Qt.Key_Down);
        keyClick(Qt.Key_Return);
        compare(combo.currentIndex, 1);
    }
}

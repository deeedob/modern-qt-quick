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

    Component {
        id: spinComponent
        SpinBox {
            editable: true
        }
    }

    function test_spinIndicatorsStayInsideBorder(): void {
        const spin = createTemporaryObject(spinComponent, testCase);
        verify(spin.down.indicator.x > 0);
        verify(spin.up.indicator.x + spin.up.indicator.width < spin.width);
    }

    function test_spinSuffixIsNotEditable(): void {
        const spin = createTemporaryObject(spinComponent, testCase, {
                                               "suffix": "px",
                                               "value": 42
                                           });
        verify(spin !== null);
        compare(spin.suffix, "px");
        spin.contentItem.selectAll();
        compare(spin.contentItem.selectedText, "42");
    }

    function test_borderAndSelection(): void {
        const combo = createTemporaryObject(comboComponent, testCase);
        compare(combo.background.border.width, 1);
        mouseClick(combo);
        tryVerify(() => combo.popup.visible);
        const item = combo.popup.contentItem.itemAtIndex(0);
        verify(item !== null);
        compare(item.text, "Plain");
        compare(item.background.color, Theme.hover);
        keyClick(Qt.Key_Down);
        keyClick(Qt.Key_Return);
        compare(combo.currentIndex, 1);
    }
}

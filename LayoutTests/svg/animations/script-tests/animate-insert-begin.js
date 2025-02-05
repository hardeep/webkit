description("Test behavior of dynamically inserting animate with begin attribute");
createSVGTestCase();

// Setup test document
var rect = createSVGElement("rect");
rect.setAttribute("id", "rect");
rect.setAttribute("x", "0");
rect.setAttribute("y", "45");
rect.setAttribute("width", "10");
rect.setAttribute("height", "10");
rect.setAttribute("fill", "green");

var animate = createSVGElement("animate");
animate.setAttribute("id", "animation");
animate.setAttribute("attributeName", "x");
animate.setAttribute("begin", "0");
animate.setAttribute("from", "0");
animate.setAttribute("to", "90");
animate.setAttribute("dur", "3s");
animate.setAttribute("fill", "freeze");
rect.appendChild(animate);
rootSVGElement.appendChild(rect);

// Setup animation test
function sample1() {
    shouldBe("rect.x.baseVal.value", "0");
}

function sample2() {
    shouldBe("rect.x.baseVal.value", "90");
}

function executeTest() {
    const expectedValues = [
        // [animationId, time, sampleCallback]
        ["animation", 0.0, sample1],
        ["animation", 3.0, sample2],
    ];

    runAnimationTest(expectedValues);
}

window.animationStartsImmediately = true;
var successfullyParsed = true;

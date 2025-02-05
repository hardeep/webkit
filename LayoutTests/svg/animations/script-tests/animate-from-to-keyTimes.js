createSVGTestCase();

// Setup test document
var rect = createSVGElement("rect");
rect.setAttribute("id", "rect");
rect.setAttribute("x", "100");
rect.setAttribute("width", "100");
rect.setAttribute("height", "100");
rect.setAttribute("fill", "green");
rect.setAttribute("onclick", "executeTest()");

var animate = createSVGElement("animate");
animate.setAttribute("id", "animation");
animate.setAttribute("attributeName", "x");
animate.setAttribute("to", "200");
animate.setAttribute("dur", "4s");
animate.setAttribute("keyTimes", "0;0.25");
animate.setAttribute("calcMode", "discrete");
animate.setAttribute("fill", "freeze");
rect.appendChild(animate);
rootSVGElement.appendChild(rect);

// Setup animation test
function sample1() {
    shouldBe("rect.x.baseVal.value", "100");
}

function sample2() {
    shouldBe("rect.x.baseVal.value", "200");
}

function executeTest() {
    const expectedValues = [
        // [animationId, time, sampleCallback]
        ["animation", 0.5, sample1],
        ["animation", 1.5, sample2],
        ["animation", 2.5, sample2]
    ];

    runAnimationTest(expectedValues);
}

window.clickX = 150;
var successfullyParsed = true;

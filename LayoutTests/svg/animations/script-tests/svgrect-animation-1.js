description("Test from-to SVGRect animation.");
createSVGTestCase();

// Setup test document
var rect = createSVGElement("rect");
rect.setAttribute("width", "100");
rect.setAttribute("height", "100");
rect.setAttribute("fill", "green");
rect.setAttribute("onclick", "executeTest()");
rootSVGElement.appendChild(rect);

rootSVGElement.setAttribute("id", "svg");
rootSVGElement.setAttribute("viewBox", "0 0 100 100");

var animate = createSVGElement("animate");
animate.setAttribute("id", "animation");
animate.setAttribute("attributeName", "viewBox");
animate.setAttribute("begin", "click");
animate.setAttribute("dur", "4s");
animate.setAttribute("from", "0 0 100 100");
animate.setAttribute("to", "50 50 50 50");
rootSVGElement.appendChild(animate);

// Setup animation test
function sample1() {
    // Check initial/end conditions
    shouldBe("rootSVGElement.viewBox.animVal.x", "0");
    shouldBe("rootSVGElement.viewBox.animVal.y", "0");
    shouldBe("rootSVGElement.viewBox.animVal.width", "100");
    shouldBe("rootSVGElement.viewBox.animVal.height", "100");

    shouldBe("rootSVGElement.viewBox.baseVal.x", "0");
    shouldBe("rootSVGElement.viewBox.baseVal.y", "0");
    shouldBe("rootSVGElement.viewBox.baseVal.width", "100");
    shouldBe("rootSVGElement.viewBox.baseVal.height", "100");
}

function sample2() {
    shouldBe("rootSVGElement.viewBox.animVal.x", "25");
    shouldBe("rootSVGElement.viewBox.animVal.y", "25");
    shouldBe("rootSVGElement.viewBox.animVal.width", "75");
    shouldBe("rootSVGElement.viewBox.animVal.height", "75");

    shouldBe("rootSVGElement.viewBox.baseVal.x", "25");
    shouldBe("rootSVGElement.viewBox.baseVal.y", "25");
    shouldBe("rootSVGElement.viewBox.baseVal.width", "75");
    shouldBe("rootSVGElement.viewBox.baseVal.height", "75");
}

function sample3() {
    shouldBeCloseEnough("rootSVGElement.viewBox.animVal.x", "50");
    shouldBeCloseEnough("rootSVGElement.viewBox.animVal.y", "50");
    shouldBeCloseEnough("rootSVGElement.viewBox.animVal.width", "50");
    shouldBeCloseEnough("rootSVGElement.viewBox.animVal.height", "50");

    shouldBeCloseEnough("rootSVGElement.viewBox.baseVal.x", "50");
    shouldBeCloseEnough("rootSVGElement.viewBox.baseVal.y", "50");
    shouldBeCloseEnough("rootSVGElement.viewBox.baseVal.width", "50");
    shouldBeCloseEnough("rootSVGElement.viewBox.baseVal.height", "50");
}

function executeTest() {
    const expectedValues = [
        // [animationId, time, sampleCallback]
        ["animation", 0.0,   sample1],
        ["animation", 2.0,   sample2],
        ["animation", 3.999, sample3],
        ["animation", 4.0,   sample1]
    ];

    runAnimationTest(expectedValues);
}

var successfullyParsed = true;

description("Test to make sure shadow nodes are not exposed.");

var container = document.createElement("p");
document.body.appendChild(container);

container.appendChild(document.createTextNode("Some text: "));

shouldBe("getSelection().anchorNode", "null");
shouldBe("getSelection().anchorOffset", "0");
shouldBe("getSelection().focusNode", "null");
shouldBe("getSelection().focusOffset", "0");
shouldBe("getSelection().isCollapsed", "true");
shouldBe("getSelection().rangeCount", "0");

shouldBe("getSelection().baseNode", "null");
shouldBe("getSelection().baseOffset", "0");
shouldBe("getSelection().extentNode", "null");
shouldBe("getSelection().extentOffset", "0");
shouldBe("getSelection().type", "'None'");

debug("\nAdd an input element.\n");

var input = document.createElement("input");
container.appendChild(input);
input.value = "text";
input.focus();
input.select();

shouldBe("getSelection().anchorNode", "container");
shouldBe("getSelection().anchorOffset", "1");
shouldBe("getSelection().focusNode", "container");
shouldBe("getSelection().focusOffset", "1");
shouldBe("getSelection().isCollapsed", "true");
shouldBe("getSelection().rangeCount", "1");
shouldBe("getSelection().getRangeAt(0).startContainer", "container");
shouldBe("getSelection().getRangeAt(0).startOffset", "1");
shouldBe("getSelection().getRangeAt(0).endContainer", "container");
shouldBe("getSelection().getRangeAt(0).endOffset", "1");

shouldBe("getSelection().baseNode", "container");
shouldBe("getSelection().baseOffset", "1");
shouldBe("getSelection().extentNode", "container");
shouldBe("getSelection().extentOffset", "1");
shouldBe("getSelection().type", "'Range'");

debug("\nAdd a textarea element.\n");

var textarea = document.createElement("textarea");
container.appendChild(textarea);
textarea.value = "text";
textarea.focus();
textarea.select();

shouldBe("getSelection().anchorNode", "container");
shouldBe("getSelection().anchorOffset", "2");
shouldBe("getSelection().focusNode", "container");
shouldBe("getSelection().focusOffset", "2");
shouldBe("getSelection().isCollapsed", "true");
shouldBe("getSelection().rangeCount", "1");
shouldBe("getSelection().getRangeAt(0).startContainer", "container");
shouldBe("getSelection().getRangeAt(0).startOffset", "2");
shouldBe("getSelection().getRangeAt(0).endContainer", "container");
shouldBe("getSelection().getRangeAt(0).endOffset", "2");

shouldBe("getSelection().baseNode", "container");
shouldBe("getSelection().baseOffset", "2");
shouldBe("getSelection().extentNode", "container");
shouldBe("getSelection().extentOffset", "2");
shouldBe("getSelection().type", "'Range'");

document.body.removeChild(container);

debug("");

var successfullyParsed = true;

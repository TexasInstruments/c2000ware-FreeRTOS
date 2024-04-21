let references = system.getScript("/kernel/freertos_tool/FREERTOSReferences.js");

exports = {
    moduleStatic: {},
    references: [
        references.getReferencePath("HEAP2"),
    ]
};
let references = system.getScript("/kernel/freertos_tool/FREERTOSReferences.js");

exports = {
    moduleStatic: {},
    references: [
        references.getReferencePath("TASK"),
        references.getReferencePath("QUEUE"),
        references.getReferencePath("LIST"),
        references.getReferencePath("TIMER"),
        references.getReferencePath("EVENTS"),
        references.getReferencePath("SBUFFER"),
        references.getReferencePath("PORT"),
        references.getReferencePath("PORTASM"),
    ]
};
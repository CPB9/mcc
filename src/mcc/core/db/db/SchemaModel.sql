

CREATE TABLE type (
    id        INTEGER NOT NULL PRIMARY KEY,
    name      TEXT    NOT NULL UNIQUE,
    unit      TEXT    NULL,
    kind      TEXT    NOT NULL,
    info      TEXT    NULL,
    base_type INTEGER NULL FOREIGN KEY REFERENCES type(id) ON DELETE RESTRICT ON UPDATE CASCADE
);

CREATE TABLE struct_field (
    id        INTEGER NOT NULL PRIMARY KEY,
    struct_id INTEGER NOT NULL FOREIGN KEY REFERENCES type(id) ON DELETE RESTRICT ON UPDATE CASCADE,
    name      TEXT    NOT NULL,
    type      INTEGER NOT NULL FOREIGN KEY REFERENCES type(id) ON DELETE RESTRICT ON UPDATE CASCADE
    info      TEXT    NULL,
    UNIQUE(struct_id, name)
);

CREATE TABLE enum_values (
    id       INTEGER NOT NULL PRIMARY KEY,
    enum_id  INTEGER NOT NULL FOREIGN KEY REFERENCES type(id) ON DELETE RESTRICT ON UPDATE CASCADE,
    name     TEXT    NOT NULL,
    value    TEXT    NOT NULL,
    UNIQUE(enum_id, name),
    UNIQUE(enum_id, value)
);

CREATE TABLE array_type (
    type_id         INTEGER NOT NULL UNIQUE FOREIGN KEY REFERENCES type(id) ON DELETE RESTRICT ON UPDATE CASCADE,
    element_type_id INTEGER NOT NULL FOREIGN KEYR EFERENCES type(id) ON DELETE RESTRICT ON UPDATE CASCADE,
    length_type_id  INTEGER NOT NULL FOREIGN KEYR EFERENCES type(id) ON DELETE RESTRICT ON UPDATE CASCADE,
    min_length      INTEGER NOT NULL,
    max_length      INTEGER NOT NULL
);

CREATE TABLE component (
    id      INTEGER NOT NULL PRIMARY KEY,
    name    TEXT    NOT NULL UNIQUE,
    type_id INTEGER NOT NULL FOREIGN KEY REFERENCES type(id) ON DELETE RESTRICT ON UPDATE CASCADE
);

CREATE TABLE command (
    id           INTEGER NOT NULL PRIMARY KEY,
    component_id INTEGER NOT NULL FOREIGN KEY REFERENCES component(id) ON DELETE RESTRICT ON UPDATE CASCADE,
    name         TEXT    NOT NULL,
    info         TEXT    NOT NULL,
    UNIQUE(component_id, name)
);

CREATE TABLE command_argument (
    id             INTEGER NOT NULL PRIMARY KEY,
    command_id     INTEGER NOT NULL FOREIGN KEY REFERENCES command(id) ON DELETE RESTRICT ON UPDATE CASCADE,
    argument_index INTEGER NOT NULL,
    name           TEXT    NOT NULL,
    type_id        INTEGER NOT NULL FOREIGN KEY REFERENCES type(id) ON DELETE RESTRICT ON UPDATE CASCADE,
    UNIQUE(command_id, argument_index),
    UNIQUE(command_id, name)
);

CREATE TABLE message (
    id            INTEGER NOT NULL PRIMARY KEY,
    component_id  INTEGER NOT NULL FOREIGN KEY REFERENCES component(id) ON DELETE RESTRICT ON UPDATE CASCADE,
    message_index INTEGER NOT NULL,
    name          TEXT    NOT NULL,
    info          TEXT    NULL,
    UNIQUE(component_id, message_index),
    UNIQUE(component_id, name)
);

CREATE TABLE message_parameter (
    id              INTEGER NOT NULL PRIMARY KEY,
    message_id      INTEGER NOT NULL FOREIGN KEY REFERENCES message(id) ON DELETE RESTRICT ON UPDATE CASCADE,
    parameter_index INTEGER NOT NULL,
    name            TEXT    NOT NULL,
    UNIQUE(message_id, parameter_index),
    UNIQUE(message_id, name)
);
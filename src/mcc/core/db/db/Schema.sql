CREATE TABLE service
(
    id      integer PRIMARY KEY NOT NULL,
    name    text    UNIQUE      NOT NULL,
    info    text                NOT NULL
);

CREATE TABLE guid
(
    id      integer PRIMARY KEY NOT NULL,
    name    text    UNIQUE      NOT NULL
);

CREATE TABLE firmware
(
    id          integer PRIMARY KEY NOT NULL,
    name        text    UNIQUE      NOT NULL,
    info        text                NOT NULL,
    registered  text                NOT NULL,
    source      text                NOT NULL
);

CREATE TABLE trait
(
    id          integer PRIMARY KEY NOT NULL,
    firmware_id integer                     , 
    parent_id   integer                     ,
    name        text                NOT NULL,
    unique_name text                NOT NULL,
    number      integer             NOT NULL,
    info        text                NOT NULL,
    guid_id     integer UNIQUE      NOT NULL,
    kind        text                NOT NULL,
    bit_size    integer,
    base_trait  integer,

    UNIQUE(firmware_id, number),
    UNIQUE(firmware_id, unique_name),
    UNIQUE(firmware_id, parent_id, name),
    
    FOREIGN KEY(parent_id)  REFERENCES trait(id),
    FOREIGN KEY(guid_id)    REFERENCES guid(id),
    FOREIGN KEY(firmware_id)REFERENCES firmware(id),
    FOREIGN KEY(base_trait) REFERENCES trait(id)
);

CREATE TABLE trait_field
(
    id          integer PRIMARY KEY NOT NULL,
    trait_id    integer             NOT NULL,
    number      integer             NOT NULL,
    type_id     integer             NOT NULL,
    name        text                NOT NULL,
    info        text                NOT NULL,
    unit        text,
    properties  text,
    value_min   double,
    value_max   double,

    UNIQUE(trait_id, number),
    UNIQUE(trait_id, name),

    FOREIGN KEY(type_id)    REFERENCES trait(id),
    FOREIGN KEY(trait_id)   REFERENCES trait(id)
);

CREATE TABLE trait_method
(
    id              integer PRIMARY KEY NOT NULL,
    trait_id        integer             NOT NULL,
    number          integer             NOT NULL,
    name            text                NOT NULL,
    info            text                NOT NULL,

    UNIQUE(trait_id, number),
    UNIQUE(trait_id, name),

    FOREIGN KEY(trait_id)   REFERENCES trait(id)
);

CREATE TABLE trait_method_arg
(
    id          integer PRIMARY KEY NOT NULL,
    method_id   integer             NOT NULL,
    number      integer             NOT NULL,
    type_id     integer             NOT NULL,
    info        text                NOT NULL,
    name        text,
    unit        text,
    value_min   double,
    value_max   double,

    UNIQUE(method_id, number),
    UNIQUE(method_id, name),

    FOREIGN KEY(method_id)    REFERENCES trait_method(id),
    FOREIGN KEY(type_id)      REFERENCES trait(id)
);

CREATE TABLE device_kind
(
    id      integer PRIMARY KEY NOT NULL,
    name    text    UNIQUE  NOT NULL,
    info    text            NOT NULL
-- icon ref?
);

CREATE TABLE device
(
    id          integer PRIMARY KEY NOT NULL,
    name        text    UNIQUE      NOT NULL,
    info        text                NOT NULL,
    kind_id     integer,
    firmware_id integer,
    registered  text                NOT NULL,
    updated     text                NOT NULL,
    
    FOREIGN KEY(kind_id)            REFERENCES device_kind(id),
    FOREIGN KEY(firmware_id)        REFERENCES firmware(id)
);

CREATE TABLE mcc_tm
(
    id          integer PRIMARY KEY NOT NULL,
    device_id   integer             NOT NULL,
    param_id    integer             NOT NULL,
    time        text                NOT NULL,
    value       text                NOT NULL,
    
    FOREIGN KEY(device_id)       REFERENCES device(id),
    FOREIGN KEY(param_id)        REFERENCES trait_field(id)
);
--(
--    id          integer PRIMARY KEY NOT NULL,
--    device_id   integer             NOT NULL,
--    trait       text                NOT NULL,
--    name        text                NOT NULL,
--    time        text                NOT NULL,
--    value       text                NOT NULL,
--    
--    FOREIGN KEY(device_id)       REFERENCES device(id)
--);

CREATE TABLE mcc_cmd
(
    id              integer PRIMARY KEY NOT NULL,
    device_id       integer             NOT NULL,
    trait           text                NOT NULL,
    name            text                NOT NULL,
    params          text                NOT NULL,
    time            text                NOT NULL,
    collation_id    integer             NOT NULL,
    
    FOREIGN KEY(device_id)       REFERENCES device(id)
);

CREATE TABLE mcc_cmd_state
(
    id              integer PRIMARY KEY NOT NULL,
    device_id       integer             NOT NULL,
    collation_id    integer             NOT NULL,
    time            text                NOT NULL,
    state           integer             NOT NULL,
    reason          text                NOT NULL,
    
    FOREIGN KEY(device_id)       REFERENCES device(id)
);

CREATE TABLE mcc_action
(
    id              integer PRIMARY KEY NOT NULL,
    time            text                NOT NULL,
    kind            text                NOT NULL,
    name            text                NOT NULL,
    action          text                NOT NULL,
    details         text                        
);

CREATE TABLE protocol
(
    id          integer PRIMARY KEY NOT NULL,
    name        text    UNIQUE      NOT NULL,
    info        text                NOT NULL,
    param_info  text                NOT NULL,
    service_id  integer             NOT NULL,
    trait_id    integer,

    FOREIGN KEY(trait_id)      REFERENCES trait(id),
    FOREIGN KEY(service_id)    REFERENCES service(id)
);

CREATE TABLE device_protocol
(
    id              integer PRIMARY KEY NOT NULL,
    device_id       integer             NOT NULL,
    protocol_id     integer             NOT NULL,
    protocol_value  integer             NOT NULL,

    UNIQUE(protocol_id, protocol_value),

    FOREIGN KEY(device_id)      REFERENCES device(id),
    FOREIGN KEY(protocol_id)    REFERENCES protocol(id)
);


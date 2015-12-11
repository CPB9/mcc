CREATE TABLE Guid
(
    Id      integer PRIMARY KEY NOT NULL,
    Guid    text    UNIQUE      NOT NULL,
    Kind    text                NOT NULL,
    Defined boolean             NOT NULL
);

CREATE TABLE Sw_Namespace
(
    Id          integer PRIMARY KEY NOT NULL,
    Name        text    UNIQUE      NOT NULL,
    Guid_id     integer UNIQUE      NOT NULL,
    Description text,
    Comment     text,
    FOREIGN KEY(Guid_id) REFERENCES Guid(Id)
);

CREATE TABLE Sw_Trait
(
    Id              integer PRIMARY KEY NOT NULL,
    Namespace_id    integer             NOT NULL,
    Name            text                NOT NULL,
    Guid_id         integer UNIQUE      NOT NULL,
    Description     text,
    Comment         text,
    UNIQUE(Namespace_id, Name),
    FOREIGN KEY(Namespace_id) REFERENCES Sw_Namespace(Id),
    FOREIGN KEY(Guid_id) REFERENCES Guid(Id)
);

CREATE TABLE Sw_TraitRelation
(
    Id          integer PRIMARY KEY NOT NULL,
    Parent_id   integer,
    Trait_id    integer             NOT NULL,
    UNIQUE(Parent_id, Trait_id)
    FOREIGN KEY(Parent_id)  REFERENCES Sw_Trait(Id),
    FOREIGN KEY(Trait_id)   REFERENCES Sw_Trait(Id)
);

CREATE TABLE Sw_Type
(
    Id              integer PRIMARY KEY NOT NULL,
    Namespace_id    integer             NOT NULL,
    Name            text    UNIQUE      NOT NULL,
    Guid_id         integer UNIQUE      NOT NULL,
    Kind            text                NOT NULL,
    BitLength       integer             NOT NULL,
    Minimum         float,
    Maximum         float,
    Description     text,
    Comment         text,
    UNIQUE(Namespace_id, Name),
    FOREIGN KEY(Guid_id)        REFERENCES Guid(Id),
    FOREIGN KEY(Namespace_id)   REFERENCES Sw_Namespace(Id)
);

CREATE TABLE Sw_TypeEnumConst
(
   Id          integer  PRIMARY KEY NOT NULL,
   Type_id     integer              NOT NULL,
   Name        text     UNIQUE      NOT NULL,
   Value       integer  UNIQUE      NOT NULL,
   Description text,
   Comment     text,
   UNIQUE(Type_id, Name),
   UNIQUE(Type_id, Value),
   FOREIGN KEY(Type_id) REFERENCES Sw_Type(Id)
);

CREATE TABLE Sw_TypeStructField
(
    Id              integer PRIMARY KEY    NOT NULL,
    Type_id         integer                NOT NULL,
    Name            text    UNIQUE         NOT NULL,
    BitOffset       integer UNIQUE         NOT NULL,
    FieldType_id    integer                NOT NULL,
    Description     text,
    Comment         text,
    UNIQUE(Type_id, Name),
    UNIQUE(Type_id, BitOffset),
    FOREIGN KEY(Type_id)        REFERENCES Sw_Type(Id),
    FOREIGN KEY(FieldType_id)   REFERENCES Sw_Type(Id)
);

CREATE TABLE Sw_Command
(
    Id          integer  PRIMARY KEY NOT NULL,
    Trait_id    integer              NOT NULL,
    Name        text                 NOT NULL,
    Description text,
    Comment     text,
    UNIQUE(Trait_id, Name),
    FOREIGN KEY(Trait_id)   REFERENCES Sw_Trait(Id)
);

CREATE TABLE Sw_CommandParam
(
    Id          integer  PRIMARY KEY NOT NULL,
    Command_id  integer              NOT NULL,
    Name        text,
    Type_id     integer              NOT NULL,
    Description text,
    Comment     text,
    UNIQUE(Command_id, Name),
    FOREIGN KEY(Command_id)  REFERENCES Sw_Command(Id),
    FOREIGN KEY(Type_id)     REFERENCES Sw_Type(Id)
);

CREATE TABLE Sw_Message
(
    Id          integer PRIMARY KEY NOT NULL,
    Trait_id    integer             NOT NULL,
    Name        text                NOT NULL,
    Kind        text                NOT NULL,
    Description text,
    Comment     text,
    UNIQUE(Trait_id, Name),
    FOREIGN KEY(Trait_id)   REFERENCES Sw_Trait(Id)
);

CREATE TABLE Sw_MessageParam
(
    Id          integer  PRIMARY KEY NOT NULL,
    Message_id  integer              NOT NULL,
    Name        text                 NOT NULL,
    Type_id     integer              NOT NULL,
    Description text,
    Comment     text,
    UNIQUE(Message_id, Name),
    FOREIGN KEY(Message_id) REFERENCES Sw_Message(Id),
    FOREIGN KEY(Type_id)    REFERENCES Sw_Type(Id)
);

CREATE TABLE Sw_Firmware
(
    Id          integer PRIMARY KEY NOT NULL,
    Name        text    UNIQUE      NOT NULL,
    Guid_id     integer UNIQUE      NOT NULL,
    Description text,
    Comment     text,
    FOREIGN KEY(Guid_id)        REFERENCES Guid(Id)
);

CREATE TABLE Sw_FirmwareObjects
(
    Id          integer PRIMARY KEY NOT NULL,
    Firmware_id integer             NOT NULL,
    Trait_id    integer             NOT NULL,
    Name        text                NOT NULL,
    Description text,
    Comment     text,
    UNIQUE(Firmware_id, Name),
    FOREIGN KEY(Firmware_id)    REFERENCES Sw_Firmware(Id),
    FOREIGN KEY(Trait_id)       REFERENCES Sw_Trait(Id)
);

CREATE TABLE Hw_Device
(
    Id              integer PRIMARY KEY NOT NULL,
    Name            text    UNIQUE      NOT NULL,
    Guid_id         integer UNIQUE      NOT NULL,
    Description     text,
    Comment         text,
    FOREIGN KEY(Guid_id)        REFERENCES Guid(Id)
);

CREATE TABLE Hw_Robot
(
    Id              integer PRIMARY KEY NOT NULL,
    Name            text                NOT NULL,
    Guid_id         integer UNIQUE      NOT NULL,
    Description     text,
    Comment         text,
    FOREIGN KEY(Guid_id)        REFERENCES Guid(Id)
);

CREATE TABLE Hw_RobotPart
(
    Id              integer PRIMARY KEY NOT NULL,
    Robot_id        integer             NOT NULL,
    Device_id       integer             NOT NULL,
    Name            text                NOT NULL,
    Firmware_id     integer             NOT NULL,
    UNIQUE(Robot_id, Name),
    UNIQUE(Robot_id, Device_id),
    FOREIGN KEY(Robot_id)        REFERENCES Hw_Robot(Id),
    FOREIGN KEY(Device_id)       REFERENCES Hw_Device(Id),
    FOREIGN KEY(Firmware_id)     REFERENCES Sw_Firmware(Id)
);

CREATE TABLE Mcc_RobotPartAccess
(
    Id  integer PRIMARY KEY NOT NULL
);

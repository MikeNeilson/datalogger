
-- assosciations for display in different interfaces
create table if not exists pin_map(name varchar(255) not null, gpio_pin int not null, purpose text not null, primary key(name,gpio_pin));
-- a sensor with it's measurement schedule
create table if not exists sensor(id serial not null primary key, name text unique, schedule text);

-- pin assignments, a sensor could have more than one
create table if not exists pin_assignment(pin int not null, sensor_id int not null,
    foreign key (pin) references pin_map(gpio_pin) on update cascade on delete cascade,
    foreign key (sensor_id) references sensor(id) on update cascade on delete cascade,
    primary key(pin,sensor_id)
);

-- configuration properties for a sensor
create table if not exists sensor_property(sensor_id int not null, key text not null , value text, type text,
    foreign key (sensor_id) references sensor(id),
    primary key(sensor_id,key));

create table if not exists config(name text not null primary key, value text);

-- actual time series data
create table if not exists timeseries(date_time bigint, sensor_id int not null , double value,
    foreign key (sensor_id) references sensor(id),
    primary key(sensor_id,date_time)
);
create table if not exists schema_history(file_name text not null primary key);

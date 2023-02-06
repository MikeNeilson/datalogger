
-- assosciations for display in different interfaces
create table if not exists pin_map(name varchar(255) not null, int gpio_pin, purpose text, primary key(name,gpio_pin));
-- a sensor with it's measurement schedule
create table if not exists sensor(id serial primary key, name text unique, schedule text);

-- pin assignments, a sensor could have more than one
create table if not exists pin_assignment(pin int references(pin_map.gpio_pin), sensor_id references(sensor.id));

-- configuration properties for a sensor
create table if not exists sensor_property(sensor_id int references(sensor.id), key text not null primary key, value text, type text);

-- actual time series data
create table if not exists timeseries(date_time bigint, sensor_id int references(sensor.id), double value);
create table if not exists schema_history(file_name text not null primary key);

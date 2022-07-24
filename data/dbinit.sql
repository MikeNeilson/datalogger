create table if not exists config(key text not null primary key, value text, type text);
create table if not exists sensor(id serial primary key, name text unique, pin int);
create table if not exists timeseries(date_time bigint, sensor_id int references sensor(id));

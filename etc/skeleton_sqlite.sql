CREATE TABLE IF NOT EXISTS  node (
  node_name text NOT NULL ,
  node_weight integer NOT NULL DEFAULT '0' ,
  PRIMARY KEY (node_name)
);

CREATE TABLE IF NOT EXISTS  recovery_type (
  rectype_id integer primary key AUTOINCREMENT ,
  rectype_short_label text NOT NULL ,
  rectype_label text NOT NULL ,
  rectype_action integer DEFAULT NULL 
);

CREATE TABLE IF NOT EXISTS  job (
  job_id integer primary key AUTOINCREMENT ,
  job_name text NOT NULL ,
  job_cmd_line text NOT NULL ,
  job_weight integer NOT NULL DEFAULT '1' ,
  job_state integer DEFAULT '0' ,
    job_node_name 
    REFERENCES node (node_name ),
    job_rectype_id 
    REFERENCES recovery_type (rectype_id )
);
CREATE INDEX IF NOT EXISTS fk_job_node on  job (job_node_name ASC);
CREATE INDEX IF NOT EXISTS fk_rectype_id on job (job_rectype_id ASC);

CREATE TABLE IF NOT EXISTS  jobs_link (
  idjobs_link integer primary key AUTOINCREMENT ,
    job_next 
    REFERENCES job (job_id ),
    job_prev 
    REFERENCES job (job_id )
);
CREATE INDEX IF NOT EXISTS fk_job_prev on jobs_link (job_prev ASC);
CREATE INDEX IF NOT EXISTS fk_job_next on jobs_link (job_next ASC);


CREATE TABLE IF NOT EXISTS  resource (
  resource_id integer primary key AUTOINCREMENT,
  resource_name text NOT NULL ,
  resource_value integer NOT NULL ,
    resource_node_name 
    REFERENCES node (node_name )
);
  CREATE INDEX IF NOT EXISTS fk_resource_node on resource (resource_node_name ASC);


CREATE TABLE IF NOT EXISTS  time_constraint (
  time_c_type integer NOT NULL ,
  time_c_value text NOT NULL ,
    time_c_job_id 
    REFERENCES job (job_id ),
  PRIMARY KEY (time_c_type, time_c_job_id) 
);
CREATE INDEX IF NOT EXISTS id_job on time_constraint (time_c_job_id ASC) ;

CREATE TABLE IF NOT EXISTS schedule_job (
sched_name text not null,
sched_id references job(job_id),
primary key (sched_id)
);

CREATE INDEX IF NOT EXISTS fk_schedule_id on schedule_job  (sched_id ASC) ;


CREATE VIEW IF NOT EXISTS get_ready_time AS
select job_id from job j
where
	j.job_id not in (
		(select time_c_job_id from time_constraint)
		and not (
			j.job_id in (select time_c_job_id from time_constraint where (time_c_type = 'at' and strftime('%H %i', time_c_value) = strftime( '%H %i', 'now')))
			or j.job_id in (select time_c_job_id from time_constraint where (time_c_type = 'before' and strftime('%H %i', time_c_value) >= strftime( '%H %i', 'now')))
			or j.job_id in (select time_c_job_id from time_constraint where (time_c_type = 'after' and strftime('%H %i', time_c_value) <= strftime( '%H %i', 'now')))
		)
		or
		not (select time_c_job_id from time_constraint)
		and
		(
			j.job_id in (select time_c_job_id from time_constraint where (time_c_type = 'at' and strftime('%H %i', time_c_value) = strftime( '%H %i', 'now')))
			or j.job_id in (select time_c_job_id from time_constraint where (time_c_type = 'before' and strftime('%H %i', time_c_value) >= strftime( '%H %i', 'now')))
			or j.job_id in (select time_c_job_id from time_constraint where (time_c_type = 'after' and strftime('%H %i', time_c_value) <= strftime( '%H %i', 'now')))
		)

	);


--CREATE VIEW IF NOT EXISTS get_ready_time AS
--select job_id from job j
--where
--	j.job_id not in (
--		(select time_c_job_id from time_constraint)
--   		and not 
--		(
--    			j.job_id in (select time_c_job_id from time_constraint where (time_c_type = 'at' and strftime('%H %i', time_c_value) = strftime( '%H %i', 'now')))
--    			or j.job_id in (select time_c_job_id from time_constraint where (time_c_type = 'before' and strftime('%H %i', time_c_value) >= strftime( '%H %i', 'now')))
--    			or j.job_id in (select time_c_job_id from time_constraint where (time_c_type = 'after' and strftime('%H %i', time_c_value) <= strftime( '%H %i', 'now')))
--		)
--		or
--		not (select time_c_job_id from time_constraint)
--		and
--		(
--    			j.job_id in (select time_c_job_id from time_constraint where (time_c_type = 'at' and strftime('%H %i', time_c_value) = strftime( '%H %i', 'now')))
--    			or j.job_id in (select time_c_job_id from time_constraint where (time_c_type = 'before' and strftime('%H %i', time_c_value) >= strftime( '%H %i', 'now')))
--    			or j.job_id in (select time_c_job_id from time_constraint where (time_c_type = 'after' and strftime('%H %i', time_c_value) <= strftime( '%H %i', 'now')))
--		)
--
--	);


CREATE VIEW IF NOT EXISTS get_ready_links AS select distinct jl.job_next AS job_id from (jobs_link jl join job j) where (jl.job_prev = j.job_id) and (j.job_state = 'succeded');

CREATE VIEW IF NOT EXISTS get_available_resource AS select resource.resource_id AS resource_id,resource.resource_name AS resource_name,resource.resource_node_name AS resource_node_name,resource.resource_value AS resource_value from resource where (resource.resource_value > 0);

-- SET autocommit = 0;



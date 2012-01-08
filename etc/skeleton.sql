CREATE  TABLE IF NOT EXISTS `node` (
  `node_name` VARCHAR(45) NOT NULL ,
  `node_weight` INT(11) NOT NULL DEFAULT '0' ,
  PRIMARY KEY (`node_name`) )
ENGINE = InnoDB
DEFAULT CHARACTER SET = latin1;

CREATE  TABLE IF NOT EXISTS `recovery_type` (
  `rectype_id` INT(11) NOT NULL AUTO_INCREMENT ,
  `rectype_short_label` VARCHAR(45) NOT NULL ,
  `rectype_label` VARCHAR(45) NOT NULL ,
  `rectype_action` ENUM('restart','stop_schedule') NULL DEFAULT NULL ,
  PRIMARY KEY (`rectype_id`) )
ENGINE = InnoDB
DEFAULT CHARACTER SET = latin1
COMMENT = '\'Deals with what to do after the end of a failed job';

CREATE  TABLE IF NOT EXISTS `job` (
  `job_id` INT(11) NOT NULL AUTO_INCREMENT ,
  `job_name` VARCHAR(45) NOT NULL ,
  `job_cmd_line` VARCHAR(45) NOT NULL ,
  `job_node_name` VARCHAR(45) NOT NULL ,
  `job_weight` INT(11) NOT NULL DEFAULT '1' ,
  `job_state` ENUM('waiting','running','succeded','failed') NULL DEFAULT 'waiting' ,
  `job_rectype_id` INT(11) NULL DEFAULT NULL ,
  PRIMARY KEY (`job_id`) ,
  INDEX `fk_job_node` (`job_node_name` ASC) ,
  INDEX `fk_rectype_id` (`job_rectype_id` ASC) ,
  CONSTRAINT `fk_job_node`
    FOREIGN KEY (`job_node_name` )
    REFERENCES `node` (`node_name` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_rectype_id`
    FOREIGN KEY (`job_rectype_id` )
    REFERENCES `recovery_type` (`rectype_id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB
AUTO_INCREMENT = 5
DEFAULT CHARACTER SET = latin1;

CREATE  TABLE IF NOT EXISTS `jobs_link` (
  `idjobs_link` INT(11) NOT NULL AUTO_INCREMENT ,
  `job_prev` INT(11) NULL DEFAULT NULL ,
  `job_next` INT(11) NULL DEFAULT NULL ,
  PRIMARY KEY (`idjobs_link`) ,
  INDEX `fk_job_prev` (`job_prev` ASC) ,
  INDEX `fk_job_next` (`job_next` ASC) ,
  CONSTRAINT `fk_job_next`
    FOREIGN KEY (`job_next` )
    REFERENCES `job` (`job_id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_job_prev`
    FOREIGN KEY (`job_prev` )
    REFERENCES `job` (`job_id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB
AUTO_INCREMENT = 9
DEFAULT CHARACTER SET = latin1;


CREATE  TABLE IF NOT EXISTS `resource` (
  `resource_id` INT(11) NOT NULL AUTO_INCREMENT COMMENT '\'Gives the remaining resources of a node' ,
  `resource_name` VARCHAR(45) NOT NULL ,
  `resource_node_name` VARCHAR(11) NOT NULL ,
  `resource_value` INT(11) NOT NULL ,
  PRIMARY KEY (`resource_id`) ,
  INDEX `fk_resource_node` (`resource_node_name` ASC) ,
  CONSTRAINT `fk_resource_node`
    FOREIGN KEY (`resource_node_name` )
    REFERENCES `node` (`node_name` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB
DEFAULT CHARACTER SET = latin1;


CREATE  TABLE IF NOT EXISTS `time_constraint` (
  `time_c_job_id` INT(11) NOT NULL COMMENT '\'Stores the time constraints' ,
  `time_c_type` ENUM('at','before','after') NOT NULL ,
  `time_c_value` TIME NOT NULL ,
  PRIMARY KEY (`time_c_type`, `time_c_job_id`) ,
  INDEX `id_job` (`time_c_job_id` ASC) ,
  CONSTRAINT `id_job`
    FOREIGN KEY (`time_c_job_id` )
    REFERENCES `job` (`job_id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB
DEFAULT CHARACTER SET = latin1;

CREATE  TABLE IF NOT EXISTS `schedule_job` (
  `sched_id` INT(11) NOT NULL AUTO_INCREMENT ,
  `sched_name` VARCHAR(45) NOT NULL ,
  PRIMARY KEY (`sched_id`) ,
  INDEX `fk_schedule_id` (`sched_id` ASC) ,
  CONSTRAINT `fk_schedule_id`
    FOREIGN KEY (`sched_id` )
    REFERENCES `job` (`job_id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB
AUTO_INCREMENT = 4
DEFAULT CHARACTER SET = latin1
COMMENT = '\'This is an abstract object';

CREATE TABLE IF NOT EXISTS `get_ready_job` (`job_id` INT, `job_name` INT, `job_cmd_line` INT, `job_node_name` VARCHAR(45), `job_weight` INT, `job_state` INT, `job_rectype_id` INT);

CREATE TABLE IF NOT EXISTS `get_ready_time` (`job_id` INT);

CREATE TABLE IF NOT EXISTS `get_ready_links` (`job_id` INT);

CREATE TABLE IF NOT EXISTS `get_ready_linkless` (`job_id` INT);

CREATE TABLE IF NOT EXISTS `get_available_resource` (`resource_id` INT, `resource_name` INT, `resource_node_name` VARCHAR(45), `resource_value` INT);

DROP TABLE IF EXISTS `get_ready_job`;
CREATE  OR REPLACE ALGORITHM=UNDEFINED   
         VIEW `get_ready_job` AS select `j`.`job_id` AS `job_id`,`j`.`job_name` AS `job_name`,`j`.`job_cmd_line` AS `job_cmd_line`,`j`.`job_node_name` AS `job_node_name`,`j`.`job_weight` AS `job_weight`,`j`.`job_state` AS `job_state`,`j`.`job_rectype_id` AS `job_rectype_id` from `job` `j`
where (
  (
    `j`.`job_id` in (select `get_ready_links`.`job_id` AS `job_id` from `get_ready_links`)
    xor
    `j`.`job_id` in (select `get_ready_linkless`.`job_id` AS `job_id` from `get_ready_linkless`)
  )
  and `j`.`job_id` in (select `j`.`job_id` AS `job_id` from `get_ready_time`)
);

DROP TABLE IF EXISTS `get_ready_time`;
CREATE  OR REPLACE ALGORITHM=UNDEFINED   
         VIEW `get_ready_time` AS select job_id from job j
where
   j.job_id not in (select time_c_job_id from time_constraint)
   xor (
    j.job_id in (select time_c_job_id from time_constraint where (time_c_type = 'at' and time_format(time_c_value,'%H %i') = time_format(now(),'%H %i')))
    or j.job_id in (select time_c_job_id from time_constraint where (time_c_type = 'before' and time_format(time_c_value,'%H %i') >= time_format(now(),'%H %i')))
    or j.job_id in (select time_c_job_id from time_constraint where (time_c_type = 'after' and time_format(time_c_value,'%H %i') <= time_format(now(),'%H %i')))
   );

DROP TABLE IF EXISTS `get_ready_links`;
CREATE  OR REPLACE ALGORITHM=UNDEFINED   
         VIEW `get_ready_links` AS select distinct `jl`.`job_next` AS `job_id` from (`jobs_link` `jl` join `job` `j`) where (`jl`.`job_prev` = `j`.`job_id`) and (`j`.`job_state` = 'succeded');

DROP TABLE IF EXISTS `get_ready_linkless`;
CREATE  OR REPLACE ALGORITHM=UNDEFINED   
         VIEW `get_ready_linkless` AS SELECT `job_id` FROM `job` WHERE `job_state` = 'waiting' AND `job_id` NOT IN ( SELECT `job_next` FROM `jobs_link` );

DROP TABLE IF EXISTS `get_available_resource`;
CREATE  OR REPLACE ALGORITHM=UNDEFINED   
         VIEW `get_available_resource` AS select `resource`.`resource_id` AS `resource_id`,`resource`.`resource_name` AS `resource_name`,`resource`.`resource_node_name` AS `resource_node_name`,`resource`.`resource_value` AS `resource_value` from `resource` where (`resource`.`resource_value` > 0);

SET autocommit = 0;


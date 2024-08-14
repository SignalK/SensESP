import Accordion from "react-bootstrap/Accordion";
import { useAccordionButton } from "react-bootstrap/AccordionButton";
import Card from "react-bootstrap/Card";
import FormCheckInput from "react-bootstrap/FormCheckInput";
import { useId, useState } from "react";

/**
 * This component is a collapsible accordion with a title in the summary and
 * a radio box at the right end of the summary.
 */

export function CheckboxAccordion({title, description, children, expanded, onExpandedChange}) {

  const handleExpandedChange = () => {
    onExpandedChange(!expanded);
  };

  function CheckToggle({ eventKey }) {
    const decoratedOnClick = useAccordionButton(eventKey, handleExpandedChange);

    const id = useId();

    return (
      <FormCheckInput
        type="checkbox"
        id={id}
        checked={expanded}
        onClick={decoratedOnClick}
        className="checkbox-lg" />
    );
  }

  return (
    <Accordion defaultActiveKey={"0"}>
      <Card>
        <Card.Header className="d-flex justify-content-between align-items-start">
          <div class="ms-2 me-auto">
            <div className="fw-bold">{title}</div>
            {description}
          </div>
          <div class="col-auto">
            <CheckToggle eventKey="0" />
          </div>
        </Card.Header>
        <Accordion.Collapse eventKey="0">
          <Card.Body>{children}</Card.Body>
        </Accordion.Collapse>
      </Card>
    </Accordion>
  );
}

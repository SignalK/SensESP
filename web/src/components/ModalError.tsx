import Modal from "bootstrap/js/dist/modal";
import { type JSX } from "preact";
import { useEffect } from "preact/hooks";

interface ModalErrorProps {
  id: string;
  title: string;
  children: React.ReactNode;
  show: boolean;
  onHide: () => void;
}

export function ModalError({
  id,
  title,
  children,
  show,
  onHide,
}: ModalErrorProps): JSX.Element {
  useEffect(() => {
    if (show) {
      const modal = Modal.getOrCreateInstance(`#${id}`);
      modal.show();
    }
  });

  return (
    <>
      <div id={id} className="modal fade" role="dialog" tabIndex={-1}>
        <div className="modal-dialog">
          <div className="modal-content">
            <div className="modal-header">
              <h5 className="modal-title">{title}</h5>
              <button
                type="button"
                className="btn-close"
                data-bs-dismiss="modal"
                onClick={onHide}
              />
            </div>
            <div className="modal-body">{children}</div>
            <div className="modal-footer">
              <button
                type="button"
                className="btn btn-primary"
                data-bs-dismiss="modal"
                onClick={onHide}
              >
                Close
              </button>
            </div>
          </div>
        </div>
      </div>
    </>
  );
}

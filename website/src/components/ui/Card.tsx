import { Link } from 'react-router-dom';

interface CardProps {
  title: string;
  description: string;
  icon?: string;
  href?: string;
  cta?: { label: string; href: string };
  className?: string;
}

export function Card({
  title,
  description,
  icon,
  href,
  cta,
  className = '',
}: CardProps) {
  const content = (
    <>
      {icon && <span className="text-2xl mb-4 block">{icon}</span>}
      <h3 className="text-text-primary font-semibold text-lg mb-2">{title}</h3>
      <p className="text-text-secondary text-sm leading-relaxed">
        {description}
      </p>
      {cta && (
        <Link
          to={cta.href}
          className="inline-flex items-center mt-4 text-accent text-sm font-medium hover:text-accent-muted transition-colors duration-150"
        >
          {cta.label}
          <span className="ml-1">→</span>
        </Link>
      )}
    </>
  );

  const base = `border border-border-subtle rounded-lg p-6 bg-surface-raised transition-colors duration-150 hover:border-accent/40 ${className}`.trim();

  if (href) {
    return (
      <Link to={href} className={`block ${base}`}>
        {content}
      </Link>
    );
  }

  return <div className={base}>{content}</div>;
}
